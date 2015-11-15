#include <queue>

#include "reg_demo.h"
#include <despot/util/coord.h>

using namespace std;

namespace despot {

/* ==============================================================================
 * RegDemoState class
 * ==============================================================================*/

RegDemoState::RegDemoState() {
}

RegDemoState::RegDemoState(int _state_id) {
	state_id = _state_id;
}

string RegDemoState::text() const {
	return "s" + to_string(state_id);
}

/* ==============================================================================
 * RegDemo class
 * ==============================================================================*/

RegDemo::RegDemo() {
	string map = string("mapSize = 7\n") + string("0 0 0 0.75 0.75 0.75 500\n");
	istringstream iss(map);
	Init(iss);
}

RegDemo::RegDemo(string params_file) {
	ifstream fin(params_file.c_str(), ifstream::in);
	Init(fin);
	fin.close();
}

void RegDemo::Init(istream& is) {
	string tok;
	is >> tok >> tok >> size_;
	states_.resize(size_ + 1);
	obs_.resize(size_ + 1);
	trap_prob_.resize(size_ + 1);
	for (int x = 0; x < size_; x++) {
		states_[x] = new RegDemoState(x);
		obs_[x] = (x == 2 ? 1 : 0);

		is >> trap_prob_[x];

		if (x == size_ - 1) {
			goal_reward_ = trap_prob_[x];
			trap_prob_[x] = 0;
		}
	}
	states_[size_] = new RegDemoState(size_); // Trapped
	obs_[size_] = 2;
	trap_prob_[size_] = 1.0; // No way to escape

	// Build transition matrix
	transition_probabilities_.resize(states_.size());
	for (int s = 0; s < states_.size(); s++) {
		transition_probabilities_[s].resize(NumActions());

		for (int a = 0; a < NumActions(); a++) {
			if (s == size_) {
				transition_probabilities_[s][a].push_back(State(s, 1.0));
			} else if (a == A_STAY) {
				if (s != size_ - 1) // Not a terminal state
					transition_probabilities_[s][a].push_back(State(s, 1.0));
			} else {
				int next = s;
				if (a == A_RIGHT)
					next = s + 1;
				if (a == A_LEFT)
					next = s - 1;

				if (next < 0)
					next = 0;
				if (next >= size_)
					next = size_ - 1;

				if (trap_prob_[next] < 1.0)
					transition_probabilities_[s][a].push_back(
						State(next, 1 - trap_prob_[next]));
				if (trap_prob_[next] > 0.0)
					transition_probabilities_[s][a].push_back(
						State(size_, trap_prob_[next]));
			}
		}
	}
}

bool RegDemo::Step(State& s, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	RegDemoState& state = static_cast<RegDemoState&>(s);

	reward =
		(state.state_id == size_ - 1 && action == A_STAY) ?
			goal_reward_ : (action == A_STAY ? 0 : -1);

	if (state.state_id == size_ - 1 && action == A_STAY)
		return true;

	const vector<State>& distribution =
		transition_probabilities_[state.state_id][action];
	double sum = 0;
	for (int i = 0; i < distribution.size(); i++) {
		const State& next = distribution[i];
		sum += next.weight;
		if (sum >= random_num) {
			state.state_id = next.state_id;
			break;
		}
	}

	obs = obs_[state.state_id];

	return false;
}

int RegDemo::NumStates() const {
	return size_ + 1;
}

double RegDemo::ObsProb(OBS_TYPE obs, const State& state, int a) const {
	return obs == obs_[state.state_id];
}

const vector<State>& RegDemo::TransitionProbability(int s, int a) const {
	return transition_probabilities_[s][a];
}

void RegDemo::PrintTransitions() const {
	cout << "Transitions (Start)" << endl;
	for (int s = 0; s < NumStates(); s++) {
		cout
			<< "--------------------------------------------------------------------------------"
			<< endl;
		cout << "State " << s << endl;
		PrintState(*GetState(s));
		for (int a = 0; a < NumActions(); a++) {
			cout << transition_probabilities_[s][a].size()
				<< " outcomes for action " << a << endl;
			for (int i = 0; i < transition_probabilities_[s][a].size(); i++) {
				const State& next = transition_probabilities_[s][a][i];
				cout << "Next = (" << next.state_id << ", " << next.weight
					<< ")" << endl;
				PrintState(*GetState(next.state_id));
			}
		}
	}
	cout << "Transitions (End)" << endl;
}

void RegDemo::PrintMDPPolicy() const {
	cout << "MDP (Start)" << endl;
	for (int s = 0; s < NumStates(); s++) {
		cout << "State " << s << "; Action = " << policy_[s].action
			<< "; Reward = " << policy_[s].value << endl;
		PrintState(*(states_[s]));
	}
	cout << "MDP (End)" << endl;
}

State* RegDemo::CreateStartState(string type) const {
	double prob = Random::RANDOM.NextDouble();
	return new RegDemoState((prob <= 0.5) ? 0 : 1);
}

Belief* RegDemo::InitialBelief(const State* start, string type) const {
	vector<State*> particles;
	for (int i = 0; i < 2; i++)
		particles.push_back(static_cast<RegDemoState*>(Allocate(i, 0.5)));

	Belief* belief = new ParticleBelief(particles, this);
	return belief;
}

ScenarioUpperBound* RegDemo::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP" || name == "DEFAULT") {
		return new MDPUpperBound(this, *this);
	} else {
		cerr << "Unsupported scenario upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

ScenarioLowerBound* RegDemo::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	const DSPOMDP* model = this;
	const StateIndexer* indexer = this;
	const StatePolicy* policy = this;
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(model);
	} else if (name == "RANDOM") {
		return new RandomPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MODE" || name == "DEFAULT") {
		ComputeDefaultActions("MDP");
		return new ModeStatePolicy(model, *indexer, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MAJORITY") {
		// ComputeDefaultActions(Globals::config.default_action);
		ComputeDefaultActions("MDP");
		return new MajorityActionPolicy(model, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario lower bound: " << name << endl;
		exit(-1);
		return NULL;
	}
}

void RegDemo::PrintState(const State& s, ostream& out) const {
	char buffer[20];
	for (int x = 0; x < size_; x++) {
		if (x == size_ - 1) {
			if (s.state_id == x)
				out << "T     ";
			else
				out << "G     ";
		} else {
			if (s.state_id == x)
				out << "R     ";
			else {
				sprintf(buffer, "%4.3f ", trap_prob_[x]);
				out << buffer;
			}
		}
	}
	out << endl;
}

void RegDemo::PrintBelief(const Belief& belief, ostream& out) const {
}

void RegDemo::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	out << obs << endl;
}

void RegDemo::PrintAction(int action, ostream& out) const {
	out << (action == 0 ? "Stay" : ((action == 1) ? "Right" : "Left")) << endl;
}

State* RegDemo::Allocate(int state_id, double weight) const {
	RegDemoState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* RegDemo::Copy(const State* particle) const {
	RegDemoState* state = memory_pool_.Allocate();
	*state = *static_cast<const RegDemoState*>(particle);
	state->SetAllocated();
	return state;
}

void RegDemo::Free(State* particle) const {
	memory_pool_.Free(static_cast<RegDemoState*>(particle));
}

int RegDemo::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

void RegDemo::ComputeDefaultActions(string type) const {
	cerr << "Default action = " << type << endl;
	if (type == "MDP") {
		const_cast<RegDemo*>(this)->ComputeOptimalPolicyUsingVI();
		int num_states = NumStates();
		default_action_.resize(num_states);

		double value = 0;
		for (int s = 0; s < num_states; s++) {
			default_action_[s] = policy_[s].action;
			value += policy_[s].value;
		}
	} else {
		cerr << "Unsupported default action type " << type << endl;
		exit(0);
	}
}

int RegDemo::GetAction(const State& state) const {
	return default_action_[GetIndex(&state)];
}

double RegDemo::Reward(int s, int action) const {
	return
		(s == size_ - 1 && action == 0) ? goal_reward_ : (action == 0 ? 0 : -1);
}

} // namespace despot
