#include "chain.h"

using namespace std;

namespace despot {

/* =============================================================================
 * ChainState class
 * =============================================================================*/

ChainState::ChainState() :
	mdp_state(0) {
}

void ChainState::Init(int num_mdp_states, int num_mdp_actions) {
	mdp_transitions_.resize(num_mdp_states);
	for (int state = 0; state < num_mdp_states; state++) {
		mdp_transitions_[state].resize(num_mdp_actions);
		for (int action = 0; action < num_mdp_actions; action++) {
			mdp_transitions_[state][action].resize(num_mdp_states);
			for (int s = 0; s < num_mdp_states; s++)
				mdp_transitions_[state][action][s] = 0;
		}
	}
}

void ChainState::SetTransition(int state, int action, vector<double> row) {
	for (int s = 0; s < row.size(); s++) {
		mdp_transitions_[state][action][s] = row[s];
	}
}

bool ChainState::IsValid() const {
	int num_mdp_states = mdp_transitions_.size(), num_mdp_actions =
		mdp_transitions_[0].size();
	for (int state1 = 0; state1 < num_mdp_states; state1++)
		for (int action = 0; action < num_mdp_actions; action++) {
			double sum = 0;
			for (int state2 = 0; state2 < num_mdp_states; state2++)
				sum += mdp_transitions_[state1][action][state2];
			if (fabs(sum - 1.0) > 1e-4)
				return false;
		}
	return true;
}

string ChainState::text() const {
	ostringstream oss;
	oss << mdp_state << endl;
	for (int s1 = 0; s1 < mdp_transitions_.size(); s1++) {
		for (int a = 0; a < mdp_transitions_[s1].size(); a++) {
			oss << s1 << " " << a << " ->";
			for (int s2 = 0; s2 < mdp_transitions_[s1][a].size(); s2++)
				oss << " " << mdp_transitions_[s1][a][s2];
			oss << endl;
		}
	}
	return oss.str();
}

/* =============================================================================
 * FullChainBelief class
 * =============================================================================*/

FullChainBelief::FullChainBelief(const DSPOMDP* model, int num_mdp_states,
	int num_mdp_actions, double alpha) :
	Belief(model),
	cur_state_(0) {
	alpha_.resize(num_mdp_states);
	for (int s = 0; s < num_mdp_states; s++) {
		alpha_[s].resize(num_mdp_actions);
		for (int a = 0; a < num_mdp_actions; a++) {
			alpha_[s][a] = vector<double>(num_mdp_states, alpha);
		}
	}
}

void FullChainBelief::Update(int action, OBS_TYPE obs) {
	int next_state = obs;
	alpha_[cur_state_][action][next_state]++;
	cur_state_ = next_state;
}

vector<State*> FullChainBelief::Sample(int num_particles) const {
	int num_mdp_states = alpha_.size(), num_mdp_actions = alpha_[0].size();

	vector<State*> samples;
	for (int i = 0; i < num_particles; i++) {
		ChainState* particle = static_cast<ChainState*>(model_->Allocate(-1,
			1.0 / num_particles));
		particle->Init(num_mdp_states, num_mdp_actions);

		particle->mdp_state = cur_state_;
		for (int state = 0; state < num_mdp_states; state++) {
			for (int action = 0; action < num_mdp_actions; action++) {
				particle->SetTransition(state, action,
					Dirichlet::Next(alpha_[state][action]));
			}
		}

		static_cast<const Chain*>(model_)->ComputeOptimalValue(*particle);
		samples.push_back(particle);
	}
	return samples;
}

Belief* FullChainBelief::MakeCopy() const {
	return new FullChainBelief(model_, alpha_.size(), alpha_[0].size(),
		alpha_[0][0][0]);
}

string FullChainBelief::text() const {
	ostringstream oss;
	for (int s1 = 0; s1 < alpha_.size(); s1++) {
		for (int a = 0; a < alpha_[s1].size(); a++) {
			oss << s1 << " " << a << " ->";
			for (int s2 = 0; s2 < alpha_[s1][a].size(); s2++)
				oss << " " << alpha_[s1][a][s2];
			oss << endl;
		}
	}
	return oss.str();
}

/* =============================================================================
 * SemiChainBelief class
 * =============================================================================*/

SemiChainBelief::SemiChainBelief(const DSPOMDP* model, int num_mdp_states,
	int num_mdp_actions) :
	Belief(model),
	cur_state_(0) {
	alpha_.resize(num_mdp_actions);
	for (int a = 0; a < num_mdp_actions; a++) {
		alpha_[a] = vector<double>(2, 1.0);
	}
}

void SemiChainBelief::Update(int action, OBS_TYPE obs) {
	int next_state = obs;

	int status =
		((action == Chain::ACTION_B && obs == 0)
			|| (action == Chain::ACTION_A && obs != 0)) ? SUCCESS : SLIP;
	alpha_[action][status]++;

	cur_state_ = next_state;
}

vector<State*> SemiChainBelief::Sample(int num_particles) const {
	int num_mdp_states = Chain::NUM_MDP_STATES, num_mdp_actions =
		model_->NumActions();

	vector<State*> samples;
	for (int i = 0; i < num_particles; i++) {
		ChainState* particle = static_cast<ChainState*>(model_->Allocate(-1,
			1.0 / num_particles));
		particle->Init(num_mdp_states, num_mdp_actions);

		// particle->mdp_state = cur_state_;
		for (int state = 0; state < num_mdp_states; state++) {
			for (int action = 0; action < num_mdp_actions; action++) {
				vector<double> probs = Dirichlet::Next(alpha_[action]);
				if (action == Chain::ACTION_A) {
					particle->SetTransition(state, action, 0, probs[SLIP]);
					particle->SetTransition(state, action,
						min(state + 1, Chain::NUM_MDP_STATES - 1),
						probs[SUCCESS]);
				} else {
					particle->SetTransition(state, action, 0, probs[SUCCESS]);
					particle->SetTransition(state, action,
						min(state + 1, Chain::NUM_MDP_STATES - 1), probs[SLIP]);
				}
			}
		}

		samples.push_back(particle);
	}
	return samples;
}

Belief* SemiChainBelief::MakeCopy() const {
	return NULL; // TODO
}

string SemiChainBelief::text() const {
	ostringstream oss;
	for (int a = 0; a < alpha_.size(); a++) {
		oss << a << " ->";
		for (int r = 0; r < alpha_[a].size(); r++)
			oss << " " << alpha_[a][r];
		oss << endl;
	}
	return oss.str();
}

/* =============================================================================
 * Chain class
 * =============================================================================*/

Chain::Chain() {
	alpha_ = 1.0;
}

Chain::Chain(string fn) {
	ifstream fin(fn.c_str(), ifstream::in);
	fin >> alpha_;
}

State* Chain::DefaultStartState() const {
	ChainState* start = new ChainState();
	start->Init(NUM_MDP_STATES, NumActions());
	start->mdp_state = INITIAL_MDP_STATE;

	for (int s = 0; s < NUM_MDP_STATES; s++) {
		// slip probabilities for A
		start->SetTransition(s, ACTION_A, 0, 0.2);
		// success probabilities for B
		start->SetTransition(s, ACTION_B, 0, 0.8);
	}
	// success probabilities for A
	start->SetTransition(0, ACTION_A, 1, 0.8);
	start->SetTransition(1, ACTION_A, 2, 0.8);
	start->SetTransition(2, ACTION_A, 3, 0.8);
	start->SetTransition(3, ACTION_A, 4, 0.8);
	start->SetTransition(4, ACTION_A, 4, 0.8);

	// slip probabilities for B
	start->SetTransition(0, ACTION_B, 1, 0.2);
	start->SetTransition(1, ACTION_B, 2, 0.2);
	start->SetTransition(2, ACTION_B, 3, 0.2);
	start->SetTransition(3, ACTION_B, 4, 0.2);
	start->SetTransition(4, ACTION_B, 4, 0.2);

	return start;
}

bool Chain::Step(State& s, double random_num, int action, double &reward,
	OBS_TYPE &obs) const {
	ChainState& state = static_cast<ChainState&>(s);
	int next = Random::GetCategory(state.GetTransition(state.mdp_state, action),
		random_num);
	reward = Reward(state.mdp_state, action, next);
	state.mdp_state = next;
	obs = state.mdp_state;

	return false;
}

int Chain::NumActions() const {
	return 2;
}
double Chain::Reward(int s1, int action, int s2) const {
	/*
	 if (action == ACTION_B)
	 return 2;
	 return s1 == 4 ? 10 : 0;
	 */
	if (s2 == 0) {
		return 2;
	} else
		return (s1 == s2 && s2 == NUM_MDP_STATES - 1) ? 10 : 0;
	//return s2 == NUM_MDP_STATES-1 ? 10 : 0;
}

double Chain::ObsProb(OBS_TYPE obs, const State& s, int action) const {
	const ChainState& state = static_cast<const ChainState&>(s);
	return state.mdp_state == obs;
}

State* Chain::CreateStartState(string type) const {
	State* start = NULL;
	if (type == "DEFAULT") {
		start = DefaultStartState();
	} else if (type == "FULL") {
		cerr << "Initial belief type to be supported: " << type << endl;
		exit(0);
		//return ExactPrior(NULL)->Sample(1)[0];
	} else {
		cerr << "Unsupported initial belief type: " << type << endl;
		exit(0);
	}
	return start;
}

Belief* Chain::InitialBelief(const State* start, string type) const {
	Belief* belief = NULL;
	if (type == "DEFAULT" || type == "FULL") {
		belief = new FullChainBelief(this, NUM_MDP_STATES, NumActions(),
			alpha_);
	} else if (type == "SEMI") {
		// belief = new SemiChainBelief(this, NUM_MDP_STATES, NumActions(), alpha_);
		cerr << "Initial belief type to be supported: " << type << endl;
	} else {
		cerr << "Unsupported initial belief type: " << type << endl;
		exit(0);
	}
	return belief;
}

class OneStepLookaheadChainParticleUpperBound: public ParticleUpperBound {
public:
	OneStepLookaheadChainParticleUpperBound(const DSPOMDP* model) {
	}

	double Value(const State& s) const {
		const ChainState& state = static_cast<const ChainState&>(s);
		if (state.mdp_state == Chain::NUM_MDP_STATES - 1)
			return 10.0 / (1 - Globals::Discount());
		return 2 + 10 * Globals::Discount() / (1 - Globals::Discount());
	}
};

class ApproxMDPChainParticleUpperBound: public ParticleUpperBound {
public:
	ApproxMDPChainParticleUpperBound() {
	}

	double Value(const State& s) const {
		const ChainState& state = static_cast<const ChainState&>(s);
		return state.policy[state.mdp_state].value;
	}
};

ScenarioUpperBound* Chain::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "LOOKAHEAD") {
		return new OneStepLookaheadChainParticleUpperBound(this);
	} else if (name == "MDP" || name == "DEFAULT") {
		return new ApproxMDPChainParticleUpperBound();
	} else {
		cerr << "Unsupported base upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

class MeanMDPScenarioLowerBound: public ScenarioLowerBound {
private:
	const Chain* chain_model_;
public:
	MeanMDPScenarioLowerBound(const Chain* model, Belief* belief = NULL) :
		ScenarioLowerBound(model, belief),
		chain_model_(model) {
	}

	ValuedAction Value(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		int num_mdp_states = chain_model_->NUM_MDP_STATES, num_mdp_actions =
			chain_model_->NumActions();

		ChainState mean;
		mean.Init(num_mdp_states, num_mdp_actions);

		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			ChainState* state = static_cast<ChainState*>(particle);
			for (int state1 = 0; state1 < num_mdp_states; state1++) {
				for (int action = 0; action < num_mdp_actions; action++) {
					for (int state2 = 0; state2 < num_mdp_states; state2++) {
						double prob1 = mean.GetTransition(state1, action,
							state2);
						double prob2 = state->GetTransition(state1, action,
							state2);
						mean.SetTransition(state1, action, state2,
							prob1 + prob2);
					}
				}
			}
		}

		for (int state1 = 0; state1 < num_mdp_states; state1++) {
			for (int action = 0; action < num_mdp_actions; action++) {
				for (int state2 = 0; state2 < num_mdp_states; state2++) {
					double prob = mean.GetTransition(state1, action, state2);
					mean.SetTransition(state1, action, state2,
						prob / particles.size());
				}
			}
		}

		chain_model_->ComputeOptimalValue(mean);
		vector<ValuedAction>& policy = mean.policy;

		double value = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			ChainState* copy = static_cast<ChainState*>(chain_model_->Copy(
				particle));

			int sim_len = 0;
			OBS_TYPE obs;
			double reward, discount = 1.0;
			int position = streams.position();
			while (!streams.Exhausted()
				&& sim_len < Globals::config.max_policy_sim_len) {
				chain_model_->Step(*copy, streams.Entry(copy->scenario_id),
					policy[copy->mdp_state].action, reward, obs);
				value += copy->weight * reward * discount;
				discount *= Globals::Discount();
				streams.Advance();
				sim_len++;
			}
			streams.position(position);

			chain_model_->Free(copy);
		}

		return ValuedAction(
			policy[static_cast<ChainState*>(particles[0])->mdp_state].action,
			value);
	}
};

class MeanMDPPolicy: public Policy {
private:
	const Chain* chain_model_;
public:
	MeanMDPPolicy(const Chain* model, ParticleLowerBound* bound, Belief* belief = NULL) :
		Policy(model, bound, belief),
		chain_model_(model) {
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		int num_mdp_states = chain_model_->NUM_MDP_STATES, num_mdp_actions =
			chain_model_->NumActions();

		ChainState* mean = static_cast<ChainState*>(chain_model_->Allocate(-1,
			1.0));
		mean->Init(num_mdp_states, num_mdp_actions);

		// mean->mdp_state = cur_state_;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			ChainState* state = static_cast<ChainState*>(particle);
			for (int state1 = 0; state1 < num_mdp_states; state1++) {
				for (int action = 0; action < num_mdp_actions; action++) {
					for (int state2 = 0; state2 < num_mdp_states; state2++) {
						double prob1 = mean->GetTransition(state1, action,
							state2);
						double prob2 = state->GetTransition(state1, action,
							state2);
						mean->SetTransition(state1, action, state2,
							prob1 + prob2);
					}
				}
			}
		}

		for (int state1 = 0; state1 < num_mdp_states; state1++) {
			for (int action = 0; action < num_mdp_actions; action++) {
				for (int state2 = 0; state2 < num_mdp_states; state2++) {
					double prob = mean->GetTransition(state1, action, state2);
					mean->SetTransition(state1, action, state2,
						prob / particles.size());
				}
			}
		}

		chain_model_->ComputeOptimalValue(*mean);

		int action =
			mean->policy[static_cast<ChainState*>(particles[0])->mdp_state].action;
		chain_model_->Free(mean);
		return action;
	}
};

ScenarioLowerBound* Chain::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(this);
	} else if (name == "RANDOM") {
		return new RandomPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "ACTION_A") {
		return new BlindPolicy(this, ACTION_A,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "ACTION_B") {
		return new BlindPolicy(this, ACTION_B,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MEAN" || name == "DEFAULT") {
		// scenario_lower_bound_ = new MeanMDPScenarioLowerBound(this);
		return new MeanMDPPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported lower bound algorithm: " << name << endl;
		exit(1);
		return NULL;
	}
}

void Chain::ComputeOptimalValue(ChainState& state) const {
	vector<ValuedAction> policy = vector<ValuedAction>(NUM_MDP_STATES);
	for (int s = 0; s < NUM_MDP_STATES; s++) {
		policy[s].value = 0;
	}

	vector<ValuedAction> next_policy = vector<ValuedAction>(NUM_MDP_STATES);
	int iter = 0;
	double diff;
	while (true) {
		for (int s = 0; s < NUM_MDP_STATES; s++) {
			next_policy[s].action = -1;
			next_policy[s].value = Globals::NEG_INFTY;

			for (int a = 0; a < NumActions(); a++) {
				double v = 0;
				for (int nexts = 0; nexts < NUM_MDP_STATES; nexts++) {
					v += state.GetTransition(s, a, nexts)
						* (Reward(s, a, nexts)
							+ Globals::Discount() * policy[nexts].value);
				}

				if (v > next_policy[s].value) {
					next_policy[s].value = v;
					next_policy[s].action = a;
				}
			}
		}

		diff = 0;
		for (int s = 0; s < NUM_MDP_STATES; s++) {
			diff += fabs(next_policy[s].value - policy[s].value);
			policy[s] = next_policy[s];
		}

		iter++;
		if (diff < 0.001)
			break;
	}

	state.policy = policy;
}

void Chain::PrintState(const State& s, ostream& out) const {
	const ChainState& state = static_cast<const ChainState&>(s);
	out << state.mdp_state << endl;
}

void Chain::PrintBelief(const Belief& belief, ostream& out) const {
}

void Chain::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	out << obs << endl;
}

void Chain::PrintAction(int action, ostream& out) const {
	if (action == ACTION_A) {
		out << "Action A" << endl;
	} else {
		out << "Action B" << endl;
	}
}

State* Chain::Allocate(int state_id, double weight) const {
	ChainState* particle = memory_pool_.Allocate();
	particle->state_id = state_id;
	particle->weight = weight;
	return particle;
}

State* Chain::Copy(const State* particle) const {
	ChainState* new_particle = memory_pool_.Allocate();
	*new_particle = *static_cast<const ChainState*>(particle);
	new_particle->SetAllocated();
	return new_particle;
}

void Chain::Free(State* particle) const {
	memory_pool_.Free(static_cast<ChainState*>(particle));
}

int Chain::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

} // namespace despot
