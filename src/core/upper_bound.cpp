#include <despot/core/upper_bound.h>
#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * ScenarioUpperBound
 * =============================================================================*/

ScenarioUpperBound::ScenarioUpperBound() {
}

ScenarioUpperBound::~ScenarioUpperBound() {
}

void ScenarioUpperBound::Init(const RandomStreams& streams) {
}

/* =============================================================================
 * ParticleUpperBound
 * =============================================================================*/

ParticleUpperBound::ParticleUpperBound() {
}

ParticleUpperBound::~ParticleUpperBound() {
}

double ParticleUpperBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	double value = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		value += particle->weight * Value(*particle);
	}
	return value;
}

/* =============================================================================
 * TrivialParticleUpperBound
 * =============================================================================*/

TrivialParticleUpperBound::TrivialParticleUpperBound(const DSPOMDP* model) :
	model_(model) {
}

TrivialParticleUpperBound::~TrivialParticleUpperBound() {
}

double TrivialParticleUpperBound::Value(const State& state) const {
	return model_->GetMaxReward() / (1 - Globals::Discount());
}

double TrivialParticleUpperBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	return State::Weight(particles) * model_->GetMaxReward() / (1 - Globals::Discount());
}

/* =============================================================================
 * LookaheadUpperBound
 * =============================================================================*/

LookaheadUpperBound::LookaheadUpperBound(const DSPOMDP* model,
	const StateIndexer& indexer, ParticleUpperBound* bound) :
	model_(model),
	indexer_(indexer),
	particle_upper_bound_(bound) {
}

void LookaheadUpperBound::Init(const RandomStreams& streams) {
	int num_states = indexer_.NumStates();
	int length = streams.Length();
	int num_particles = streams.NumStreams();

	SetSize(bounds_, num_particles, length + 1, num_states);

	clock_t start = clock();
	for (int p = 0; p < num_particles; p++) {
		if (p % 10 == 0)
			cerr << p << " scenarios done! ["
				<< (double(clock() - start) / CLOCKS_PER_SEC) << "s]" << endl;
		for (int t = length; t >= 0; t--) {
			if (t == length) { // base case
				for (int s = 0; s < num_states; s++) {
					bounds_[p][t][s] = particle_upper_bound_->Value(*indexer_.GetState(s));
				}
			} else { // lookahead
				for (int s = 0; s < num_states; s++) {
					double best = Globals::NEG_INFTY;

					for (int a = 0; a < model_->NumActions(); a++) {
						double reward = 0;
						State* copy = model_->Copy(indexer_.GetState(s));
						bool terminal = model_->Step(*copy, streams.Entry(p, t),
							a, reward);
						model_->Free(copy);
						reward += (!terminal) * Globals::Discount()
							* bounds_[p][t + 1][indexer_.GetIndex(copy)];

						if (reward > best)
							best = reward;
					}

					bounds_[p][t][s] = best;
				}
			}
		}
	}
}

double LookaheadUpperBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	double bound = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		bound +=
			particle->weight
				* bounds_[particle->scenario_id][streams.position()][indexer_.GetIndex(
					particle)];
	}
	return bound;
}


/* =============================================================================
 * BeliefUpperBound
 * =============================================================================*/

BeliefUpperBound::BeliefUpperBound() {
}

BeliefUpperBound::~BeliefUpperBound() {
}

TrivialBeliefUpperBound::TrivialBeliefUpperBound(const DSPOMDP* model) :
	model_(model) {
}

double TrivialBeliefUpperBound::Value(const Belief* belief) const {
	return model_->GetMaxReward() / (1 - Globals::Discount());
}

/* =============================================================================
 * MDPUpperBound
 * =============================================================================*/

MDPUpperBound::MDPUpperBound(const MDP* model,
	const StateIndexer& indexer) :
	model_(model),
	indexer_(indexer) {
	const_cast<MDP*>(model_)->ComputeOptimalPolicyUsingVI();
	policy_ = model_->policy();
}

double MDPUpperBound::Value(const State& state) const {
	return policy_[indexer_.GetIndex(&state)].value;
}

double MDPUpperBound::Value(const Belief* belief) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double value = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		value += particle->weight * policy_[indexer_.GetIndex(particle)].value;
	}
	return value;
}

} // namespace despot
