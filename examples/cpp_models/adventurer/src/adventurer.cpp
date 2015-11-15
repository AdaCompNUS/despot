#include <queue>

#include "adventurer.h"
#include <despot/util/coord.h>

using namespace std;

namespace despot {

/* ==============================================================================
 * AdventurerState class
 * ==============================================================================*/

AdventurerState::AdventurerState() {
}

AdventurerState::AdventurerState(int _state_id) {
	state_id = _state_id;
}

string AdventurerState::text() const {
	return "s" + to_string(state_id) + "-" + to_string(state_id / Adventurer::current_->size_);
}

/* ==============================================================================
 * Adventurer class
 * ==============================================================================*/

Adventurer* Adventurer::current_ = NULL;

Adventurer::Adventurer(int num_goals) {
	current_ = this;

	if (num_goals != 2 && num_goals != 50) {
		cerr << "[Adventurer::Adventurer] Only 2 or 50 goals are supported." << endl;
		exit(0);
	}

	string map =
		num_goals == 2 ?
		(string("mapSize = 5 nGoal = 2\n")
			+ string("0.5 0.5 0.5 0.5 0.5\n") // probabilities of being trapped
			+ string("0.5 101 0.5 150\n") // goal probability, goal reward
			+ string("0.3") // observation noise
		)
		:
		(string("mapSize = 5 nGoal = 50\n")
			+ string("0.5 0.5 0.5 0.5 0.5\n") // probabilities of being trapped
			+ string("0.02 101 0.02 102 0.02 103 0.02 104 0.02 105 0.02 106 0.02 107 0.02 108 0.02 109 0.02 110 0.02 111 0.02 112 0.02 113 0.02 114 0.02 115 0.02 116 0.02 117 0.02 118 0.02 119 0.02 120 0.02 121 0.02 122 0.02 123 0.02 124 0.02 125 0.02 126 0.02 127 0.02 128 0.02 129 0.02 130 0.02 131 0.02 132 0.02 133 0.02 134 0.02 135 0.02 136 0.02 137 0.02 138 0.02 139 0.02 140 0.02 141 0.02 142 0.02 143 0.02 144 0.02 145 0.02 146 0.02 147 0.02 148 0.02 149 0.02 150\n")  // goal probability, goal reward
			+ string("0.3") // observation noise
		);

	istringstream iss(map);
	Init(iss);
	// PrintPOMDPX();
}

Adventurer::Adventurer(string params_file) {
	current_ = this;

	ifstream fin(params_file.c_str(), ifstream::in);
	Init(fin);
	fin.close();
}

void Adventurer::Init(istream& is) {
	string tok;
	is >> tok >> tok >> size_ >> tok >> tok >> num_goals_;
	states_.resize(num_goals_ * size_);
	trap_prob_.resize(size_ + 1);
	for (int x = 0; x < size_; x++) {
		for (int g = 0; g < num_goals_; g++)
			states_[g * size_ + x] = new AdventurerState(g * size_ + x);

		is >> trap_prob_[x];
	}

	max_goal_reward_ = Globals::NEG_INFTY;
	for (int i = 0; i < num_goals_; i++) {
		double prob, reward;
		is >> prob >> reward;
		goal_prob_.push_back(prob);
		goal_reward_.push_back(reward);

		if (reward > max_goal_reward_)
			max_goal_reward_ = reward;
	}

	is >> obs_noise_;

	// Build transition matrix
	transition_probabilities_.resize(states_.size());
	for (int s = 0; s < states_.size(); s++) {
		transition_probabilities_[s].resize(NumActions());

		int position = s % size_;
		int goal_type = s / size_;

		for (int a = 0; a < NumActions(); a++) {
			if (a == A_STAY) {
				if (position < size_ - 1)
					transition_probabilities_[s][a].push_back(State(s, 1.0));
			} else {
				int next_pos = (a == A_RIGHT) ? (position + 1) : (position - 1);

				if (next_pos < 0)
					next_pos = 0;
				if (next_pos >= size_)
					next_pos = size_ - 1;

				if (trap_prob_[position] < 1.0)
					transition_probabilities_[s][a].push_back(
						State(goal_type * size_ + next_pos,
							1 - trap_prob_[position]));
				// transition_probabilities_[s][a].push_back(State(next_pos, 1 - trap_prob_[position]));
			}
		}
	}

	// PrintTransitions();
}

bool Adventurer::Step(State& s, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	AdventurerState& state = static_cast<AdventurerState&>(s);

	reward = 0.0;
	int position = state.state_id % size_;
	int goal_type = state.state_id / size_;

	if (position == size_ - 1 && action == A_STAY) {
		reward = goal_reward_[goal_type];
		return true;
	}

	bool terminal = true;
	const vector<State>& distribution =
		transition_probabilities_[state.state_id][action];
	double sum = 0;
	for (int i = 0; i < distribution.size(); i++) {
		const State& next = distribution[i];
		sum += next.weight;
		if (sum >= random_num) {
			terminal = false;
			state.state_id = next.state_id;

			random_num = (sum - random_num) / next.weight;
			break;
		}
	}

	if (terminal) {
		reward = -10;
		return true;
	}

	if (random_num >= obs_noise_) {
		obs = goal_type;
	} else {
		int index = int(random_num / (obs_noise_ / (num_goals_ - 1)));
		obs = (1 + index + goal_type) % num_goals_;
	}

	return false;
}

int Adventurer::NumStates() const {
	return num_goals_ * size_;
}

double Adventurer::ObsProb(OBS_TYPE obs, const State& s, int a) const {
	const AdventurerState& state = static_cast<const AdventurerState&>(s);
	int goal_type = state.state_id / size_;
	if (obs == goal_type)
		return 1 - obs_noise_;
	return
		(obs < num_goals_) ? (obs_noise_ / (num_goals_ - 1)) : 0.0;
}

const vector<State>& Adventurer::TransitionProbability(int s, int a) const {
	return transition_probabilities_[s][a];
}

void Adventurer::PrintTransitions() const {
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

void Adventurer::PrintMDPPolicy() const {
	cout << "MDP (Start)" << endl;
	for (int s = 0; s < NumStates(); s++) {
		cout << "State " << s << "; Action = " << policy_[s].action
			<< "; Reward = " << policy_[s].value << endl;
		PrintState(*(states_[s]));
	}
	cout << "MDP (End)" << endl;
}

State* Adventurer::CreateStartState(string type) const {
	double prob = Random::RANDOM.NextDouble();
	int goal = 0;
	double sum = 0;
	for (; goal < num_goals_; goal++) {
		sum += goal_prob_[goal];
		if (sum >= prob)
			break;
	}
	AdventurerState* state = new AdventurerState(goal * size_);
	return state;
}

Belief* Adventurer::InitialBelief(const State* start, string type) const {
	vector<State*> particles;
	for (int goal = 0; goal < num_goals_; goal++) {
		AdventurerState* particle = static_cast<AdventurerState*>(Allocate(
			goal * size_, goal_prob_[goal]));
		particles.push_back(particle);
	}

	Belief* belief = new ParticleBelief(particles, this);
	return belief;
}

ParticleUpperBound* Adventurer::CreateParticleUpperBound(string name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP") {
		return new MDPUpperBound(this, *this);
	} else {
		cerr << "Unsupported particle lower bound: " << name << endl;
		exit(1);
	}
}

ScenarioUpperBound* Adventurer::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP") {
		return new MDPUpperBound(this, *this);
	} else if (name == "LOOKAHEAD") {
		return new LookaheadUpperBound(this, *this,
			CreateParticleUpperBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

class AdventurerSmartPolicy: public Policy {
private:
	const Adventurer* regdemo_model_;
public:
	AdventurerSmartPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound),
		regdemo_model_(static_cast<const Adventurer*>(model)) {
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		bool at_goal = true;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			if (static_cast<AdventurerState*>(particle)->state_id
				% regdemo_model_->size_ != regdemo_model_->size_ - 1) {
				at_goal = false;
			}
		}

		if (at_goal) {
			return regdemo_model_->A_STAY;
		}
		return regdemo_model_->A_RIGHT;
	}
};

ScenarioLowerBound* Adventurer::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	const DSPOMDP* model = this;
	const StateIndexer* indexer = this;
	const StatePolicy* policy = this;
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(model);
	} else if (name == "STAY" || name == "DEFAULT") {
		return new BlindPolicy(model, A_STAY,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "SMART") {
		return new AdventurerSmartPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "RANDOM") {
		return new RandomPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MODE") {
		const_cast<Adventurer*>(this)->ComputeDefaultActions("MDP");
		return new ModeStatePolicy(model, *indexer, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MAJORITY") {
		// ComputeDefaultActions(Globals::config.default_action);
		const_cast<Adventurer*>(this)->ComputeDefaultActions("MDP");
		return new MajorityActionPolicy(model, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario lower bound: " << name << endl;
		exit(-1);
		return NULL;
	}
}

void Adventurer::PrintState(const State& s, ostream& out) const {
	const AdventurerState& state = static_cast<const AdventurerState&>(s);

	char buffer[20];
	for (int x = 0; x < size_; x++) {
		if (x == size_ - 1) {
			if (state.state_id % size_ == x) {
				sprintf(buffer, "%s%-2.2d", "T", state.state_id / size_);
				out << buffer;
			} else {
				sprintf(buffer, "%s%-2.2d", "G", state.state_id / size_);
				out << buffer;
			}
		} else {
			if (state.state_id % size_ == x)
				out << "R     ";
			else {
				sprintf(buffer, "%4.3f ", trap_prob_[x]);
				out << buffer;
			}
		}
	}
	out << endl;
}

void Adventurer::PrintBelief(const Belief& belief, ostream& out) const {
}

void Adventurer::PrintObs(const State& state, OBS_TYPE obs,
	ostream& out) const {
	out << obs << endl;
}

void Adventurer::PrintAction(int action, ostream& out) const {
	out << (action == 0 ? "Stay" : ((action == 2) ? "Right" : "Left")) << endl;
}

State* Adventurer::Allocate(int state_id, double weight) const {
	AdventurerState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* Adventurer::Copy(const State* particle) const {
	AdventurerState* state = memory_pool_.Allocate();
	*state = *static_cast<const AdventurerState*>(particle);
	state->SetAllocated();
	return state;
}

void Adventurer::Free(State* particle) const {
	memory_pool_.Free(static_cast<AdventurerState*>(particle));
}

int Adventurer::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

void Adventurer::ComputeDefaultActions(string type) {
	cerr << "Default action = " << type << endl;
	if (type == "MDP") {
		ComputeOptimalPolicyUsingVI();
		int num_states = NumStates();
		default_action_.resize(num_states);

		double value = 0;
		for (int s = 0; s < num_states; s++) {
			default_action_[s] = policy_[s].action;
			value += policy_[s].value;
			/*
			 cout << "State " << s << endl;
			 PrintState(*states_[s]);
			 cout << "Action " << Compass::CompassString[policy_[s].action] << endl;
			 */
		}
		cerr << "MDP upper bound " << policy_[0].value << endl;
	} else {
		cerr << "Unsupported default action type " << type << endl;
		exit(0);
	}
}

int Adventurer::GetAction(const State& state) const {
	return default_action_[GetIndex(&state)];
}

double Adventurer::Reward(int s, int action) const {
	if (action == A_STAY)
		return (s % size_ == size_ - 1) ? goal_reward_[s / size_] : 0;
	return trap_prob_[s % size_] * (-10);
}

Belief* Adventurer::Tau(const Belief* belief, int action, OBS_TYPE obs) const {
	static vector<double> probs = vector<double>(NumStates());

	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		AdventurerState* state = static_cast<AdventurerState*>(particles[i]);
		const vector<State>& distribution = transition_probabilities_[GetIndex(
			state)][action];
		for (int j = 0; j < distribution.size(); j++) {
			const State& next = distribution[j];
			double p = state->weight * next.weight
				* ObsProb(obs, *(states_[next.state_id]), action);
			probs[next.state_id] += p;
			sum += p;
		}
	}

	vector<State*> new_particles;
	for (int i = 0; i < NumStates(); i++) {
		if (probs[i] > 0) {
			State* new_particle = Copy(states_[i]);
			new_particle->weight = probs[i] / sum;
			new_particles.push_back(new_particle);
			probs[i] = 0;
		}
	}

	return new ParticleBelief(new_particles, this, NULL, false);
}

void Adventurer::Observe(const Belief* belief, int action,
	map<OBS_TYPE, double>& obss) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();
	for (int i = 0; i < particles.size(); i++) {
		AdventurerState* state = static_cast<AdventurerState*>(particles[i]);
		const vector<State>& distribution = transition_probabilities_[GetIndex(
			state)][action];
		for (int j = 0; j < distribution.size(); j++) {
			const State& next = distribution[j];
			for (OBS_TYPE obs = 0; obs < num_goals_; obs++) {
				double p = state->weight * next.weight
					* ObsProb(obs, next, action);
				obss[obs] += p;
			}
		}
	}
}

double Adventurer::StepReward(const Belief* belief, int action) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		AdventurerState* state = static_cast<AdventurerState*>(particle);
		sum += state->weight * Reward(state->state_id, action);
	}

	return sum;
}

class AdventurerPOMCPPrior: public POMCPPrior {
private:
	const Adventurer* reg_model_;
public:
	AdventurerPOMCPPrior(const DSPOMDP* model) :
		POMCPPrior(model),
		reg_model_(static_cast<const Adventurer*>(model)) {
	}

	void ComputePreference(const State& state) {
		for (int a = 0; a < 3; a++) {
			legal_actions_.push_back(a);
		}

		preferred_actions_.push_back(reg_model_->A_RIGHT);
	}
};

POMCPPrior* Adventurer::CreatePOMCPPrior(string name) const {
	if (name == "UNIFORM") {
		return new UniformPOMCPPrior(this);
	} else if (name == "DEFAULT" || name == "RIGHT") {
		return new AdventurerPOMCPPrior(this);
	} else {
		cerr << "Unsupported POMCP prior: " << name << endl;
		exit(1);
		return NULL;
	}
}

void Adventurer::PrintPOMDPX() const {
	vector<string> actions;
	actions.push_back("stay");
	actions.push_back("left");
	actions.push_back("right");

	cout << "<?xml version='1.0' encoding='ISO-8859-1'?>" << endl
		<< "<pomdpx version='1.0' id='tagxmlfac' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='pomdpx.xsd'>"
		<< endl << "<Description>" << endl << "description" << endl
		<< "</Description>" << endl << "<Discount>0.95</Discount>" << endl;

	cout << "<Variable>" << endl
		<< "<StateVar vnamePrev=\"state_0\" vnameCurr=\"state_1\" fullyObs=\"false\"> <ValueEnum>"
		<< endl;
	for (int s = 0; s < NumStates(); s++)
		cout << " s" << s;
	cout << " st </ValueEnum> </StateVar>" << endl << endl
		<< "<ObsVar vname=\"obs\"> <ValueEnum>" << endl;
	for (int g = 0; g < num_goals_; g++)
		cout << " o" << g;
	cout << " ot" << endl << "</ValueEnum>" << endl << "</ObsVar>" << endl
		<< "<ActionVar vname=\"action\"> <ValueEnum> stay left right </ValueEnum> </ActionVar>"
		<< endl << "<RewardVar vname=\"reward\"/> </Variable>" << endl;

	cout
		<< "<InitialStateBelief> <CondProb> <Var> state_0 </Var> <Parent> null </Parent>"
		<< endl << "<Parameter type = \"TBL\">" << endl;
	for (int goal = 0; goal < num_goals_; goal++) {
		cout << "<Entry> <Instance> s" << goal * size_
			<< " </Instance> <ProbTable> " << 1.0 / num_goals_
			<< " </ProbTable> </Entry>" << endl;
	}
	cout << "</Parameter> </CondProb> </InitialStateBelief>" << endl;

	cout
		<< "<StateTransitionFunction> <CondProb> <Var> state_1 </Var> <Parent>action state_0</Parent>"
		<< endl << "<Parameter type = \"TBL\">" << endl;
	for (int s = 0; s < NumStates(); s++) {
		for (int a = 0; a < NumActions(); a++) {
			double sum = 0;
			for (int i = 0; i < transition_probabilities_[s][a].size(); i++) {
				const State& next = transition_probabilities_[s][a][i];
				cout << "<Entry> <Instance> " << actions[a] << " s" << s << " s"
					<< next.state_id << " </Instance> <ProbTable> "
					<< next.weight << " </ProbTable> </Entry>" << endl;
				sum += next.weight;
			}

			if (fabs(sum - 1.0) > 0.000001)
				cout << "<Entry> <Instance> " << actions[a] << " s" << s
					<< " st </Instance> <ProbTable> " << (1 - sum)
					<< " </ProbTable> </Entry>" << endl;
		}
	}
	cout
		<< "<Entry> <Instance> * st st </Instance> <ProbTable> 1.0 </ProbTable> </Entry>"
		<< endl;
	cout << "</Parameter> </CondProb> </StateTransitionFunction>" << endl;

	cout
		<< "<ObsFunction> <CondProb> <Var> obs </Var> <Parent>action state_1</Parent>"
		<< endl << "<Parameter type = \"TBL\">" << endl;
	for (int a = 0; a < NumActions(); a++) {
		for (int s = 0; s < NumStates(); s++) {
			for (int o = 0; o < num_goals_; o++) {
				cout << "<Entry> <Instance> " << actions[a] << " s" << s << " o"
					<< o << " </Instance> <ProbTable> "
					<< ObsProb(o, *(states_[s]), a) << " </ProbTable> </Entry>"
					<< endl;
			}
		}
	}
	cout
		<< "<Entry> <Instance> * st ot </Instance> <ProbTable> 1.0 </ProbTable> </Entry>"
		<< endl;
	cout << "</Parameter> </CondProb> </ObsFunction>" << endl;

	cout
		<< "<RewardFunction> <Func> <Var> reward </Var> <Parent>action state_0</Parent>"
		<< endl << "<Parameter type = \"TBL\">" << endl;
	for (int a = 0; a < NumActions(); a++) {
		for (int s = 0; s < NumStates(); s++) {
			cout << "<Entry> <Instance> " << actions[a] << " s" << s
				<< " </Instance> <ValueTable> " << Reward(s, a)
				<< " </ValueTable> </Entry>" << endl;
		}
	}
	cout
		<< "<Entry> <Instance> * st </Instance> <ValueTable> 0 </ValueTable> </Entry>"
		<< endl;
	cout << "</Parameter> </Func> </RewardFunction>" << endl;
	cout << "</pomdpx>" << endl;
}

} // namespace despot
