#include <despot/core/policy.h>
#include <despot/core/pomdp.h>
#include <unistd.h>

using namespace std;

namespace despot {

/* =============================================================================
 * Policy class
 * =============================================================================*/

Policy::Policy(const DSPOMDP* model, ParticleLowerBound* particle_lower_bound,
		Belief* belief) :
	ScenarioLowerBound(model, belief),
	particle_lower_bound_(particle_lower_bound) {
	assert(particle_lower_bound_ != NULL);
}

Policy::~Policy() {
}

ValuedAction Policy::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	vector<State*> copy;
	for (int i = 0; i < particles.size(); i++)
		copy.push_back(model_->Copy(particles[i]));

	initial_depth_ = history.Size();
	ValuedAction va = RecursiveValue(copy, streams, history);

	for (int i = 0; i < copy.size(); i++)
		model_->Free(copy[i]);

	return va;
}

ValuedAction Policy::RecursiveValue(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	if (streams.Exhausted()
		|| (history.Size() - initial_depth_
			>= Globals::config.max_policy_sim_len)) {
		return particle_lower_bound_->Value(particles);
	} else {
		int action = Action(particles, streams, history);

		double value = 0;

		map<OBS_TYPE, vector<State*> > partitions;
		OBS_TYPE obs;
		double reward;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			bool terminal = model_->Step(*particle,
				streams.Entry(particle->scenario_id), action, reward, obs);

			value += reward * particle->weight;

			if (!terminal) {
				partitions[obs].push_back(particle);
			}
		}

		for (map<OBS_TYPE, vector<State*> >::iterator it = partitions.begin();
			it != partitions.end(); it++) {
			OBS_TYPE obs = it->first;
			history.Add(action, obs);
			streams.Advance();
			ValuedAction va = RecursiveValue(it->second, streams, history);
			value += Globals::Discount() * va.value;
			streams.Back();
			history.RemoveLast();
		}

		return ValuedAction(action, value);
	}
}

void Policy::Reset() {
}

ParticleLowerBound* Policy::particle_lower_bound() const {
	return particle_lower_bound_;
}

ValuedAction Policy::Search() {
	RandomStreams streams(Globals::config.num_scenarios,
		Globals::config.search_depth);
	vector<State*> particles = belief_->Sample(Globals::config.num_scenarios);

	int action = Action(particles, streams, history_);
	double dummy_value = Globals::NEG_INFTY;

	for (int i = 0; i < particles.size(); i++)
		model_->Free(particles[i]);

	return ValuedAction(action, dummy_value);
}

/* =============================================================================
 * BlindPolicy class
 * =============================================================================*/

BlindPolicy::BlindPolicy(const DSPOMDP* model, int action, ParticleLowerBound* 
	bound, Belief* belief) :
	Policy(model, bound, belief),
	action_(action) {
}

int BlindPolicy::Action(const vector<State*>& particles, RandomStreams& streams,
	History& history) const {
	return action_;
}

ValuedAction BlindPolicy::Search() {
	double dummy_value = Globals::NEG_INFTY;
	return ValuedAction(action_, dummy_value);
}

void BlindPolicy::Update(int action, OBS_TYPE obs) {
}

/* =============================================================================
 * RandomPolicy class
 * =============================================================================*/

RandomPolicy::RandomPolicy(const DSPOMDP* model, ParticleLowerBound* bound,
	Belief* belief) :
	Policy(model, bound, belief) {
}

RandomPolicy::RandomPolicy(const DSPOMDP* model,
	const vector<double>& action_probs,
	ParticleLowerBound* bound, Belief* belief) :
	Policy(model, bound, belief),
	action_probs_(action_probs) {
	double sum = 0;
	for (int i = 0; i < action_probs.size(); i++)
		sum += action_probs[i];
	assert(fabs(sum - 1.0) < 1.0e-8);
}

int RandomPolicy::Action(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	if (action_probs_.size() > 0) {
		return Random::GetCategory(action_probs_, Random::RANDOM.NextDouble());
	} else {
		return Random::RANDOM.NextInt(model_->NumActions());
	}
}

ValuedAction RandomPolicy::Search() {
	double dummy_value = Globals::NEG_INFTY;
	if (action_probs_.size() > 0) {
		return ValuedAction(
			Random::GetCategory(action_probs_, Random::RANDOM.NextDouble()),
			dummy_value);
	} else {
		return ValuedAction(Random::RANDOM.NextInt(model_->NumActions()),
			dummy_value);
	}
}

void RandomPolicy::Update(int action, OBS_TYPE obs) {
}

/* =============================================================================
 * MajorityActionPolicy class
 * =============================================================================*/

MajorityActionPolicy::MajorityActionPolicy(const DSPOMDP* model,
	const StatePolicy& policy, ParticleLowerBound* bound, Belief* belief) :
	Policy(model, bound, belief),
	policy_(policy) {
}

int MajorityActionPolicy::Action(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	vector<double> frequencies(model_->NumActions());

	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		int action = policy_.GetAction(*particle);
		frequencies[action] += particle->weight;
	}

	int bestAction = 0;
	double bestWeight = frequencies[0];
	for (int a = 1; a < frequencies.size(); a++) {
		if (bestWeight < frequencies[a]) {
			bestWeight = frequencies[a];
			bestAction = a;
		}
	}

	return bestAction;
}

/* =============================================================================
 * ModeStatePolicy class
 * =============================================================================*/

ModeStatePolicy::ModeStatePolicy(const DSPOMDP* model,
	const StateIndexer& indexer, const StatePolicy& policy,
	ParticleLowerBound* bound, Belief* belief) :
	Policy(model, bound, belief),
	indexer_(indexer),
	policy_(policy) {
	state_probs_.resize(indexer_.NumStates());
}

int ModeStatePolicy::Action(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	double maxWeight = 0;
	State* mode = NULL;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		int id = indexer_.GetIndex(particle);
		state_probs_[id] += particle->weight;

		if (state_probs_[id] > maxWeight) {
			maxWeight = state_probs_[id];
			mode = particle;
		}
	}

	for (int i = 0; i < particles.size(); i++) {
		state_probs_[indexer_.GetIndex(particles[i])] = 0;
	}

	assert(mode != NULL);
	return policy_.GetAction(*mode);
}

/* =============================================================================
 * MMAPStatePolicy class
 * =============================================================================*/

MMAPStatePolicy::MMAPStatePolicy(const DSPOMDP* model,
	const MMAPInferencer& inferencer, const StatePolicy& policy,
	ParticleLowerBound* bound, Belief* belief) :
	Policy(model, bound, belief),
	inferencer_(inferencer),
	policy_(policy) {
}

int MMAPStatePolicy::Action(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	return policy_.GetAction(*inferencer_.GetMMAP(particles));
}

} // namespace despot
