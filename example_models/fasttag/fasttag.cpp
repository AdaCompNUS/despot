#include <queue>

#include "fasttag.h"
// #include "tag.h"

using namespace std;

double FastTag::TAG_REWARD = 10;

/*---------------------------------------------------------------------------*/

FastTagState::FastTagState() {}

FastTagState::FastTagState(int _state_id) {
	state_id = _state_id;
}

string FastTagState::text() const {
	// return concat("id = ", state_id);
	return "id = " + to_string(state_id);
}

/*---------------------------------------------------------------------------*/

void FastTag::ReadConfig(istream& is) {
	int rob = -1, opp = -1;

	string line, key, val;
  while (is >> key >> val) {
    if (key == "mapSize") {
			int nrows, ncols;
			is >> nrows >> ncols;
			floor_ = Floor(nrows, ncols);

      for (int y = 0; y < nrows; y++) {
        is >> line;
        for (int x = 0; x < ncols; x++) {
					if (line[x] != '#') {
						floor_.AddCell(Coord(x, y));
					}

					if (line[x] == 'R' || line[x] == 'Q') {
						rob = floor_.GetIndex(Coord(x, y));
					}
					if (line[x] == 'O' || line[x] == 'Q') {
						opp = floor_.GetIndex(Coord(x, y));
          }
        }
      }

			floor_.ComputeDistances();
    }
	}

	start_state_ = static_cast<FastTagState*>(Allocate(-1, 0.0));
	if (rob != -1 && opp != -1)
		start_state_->state_id = RobOppIndicesToStateIndex(rob, opp);
}

FastTag::FastTag() {
	string map = string("mapSize = 5 10\n")
		+ string("#####...##\n")
		+ string("#####...##\n")
		+ string("#####...##\n")
		+ string("...........\n")
		+ string("...........");
	istringstream iss(map);
	Init(iss);
	start_state_ = static_cast<FastTagState*>(RandomStartState());
}

FastTag::FastTag(string params_file) {
  ifstream fin(params_file.c_str(), ifstream::in);
	Init(fin);
  fin.close();
}

void FastTag::Init(istream& is) {
	ReadConfig(is);
	same_loc_obs_ = floor_.NumCells();

	FastTagState* state;
	states_.resize(NumStates());
	rob_.resize(NumStates());
	opp_.resize(NumStates());
	obs_.resize(NumStates());
	for (int rob=0; rob<floor_.NumCells(); rob++) {
		for (int opp=0; opp<floor_.NumCells(); opp++) {
			int s = RobOppIndicesToStateIndex(rob, opp);
			state = new FastTagState(s);
			states_[s] = state;
			rob_[s] = rob;
			opp_[s] = opp;
			obs_[s] = (rob == opp ? same_loc_obs_ : rob);
		}
	}

	// Build transition matrix
	transition_probabilities_.resize(NumStates());
	for (int s=0; s<NumStates(); s++) {
		transition_probabilities_[s].resize(NumActions());

		const map<int, double>& opp_distribution = OppTransitionDistribution(s);
		for (int a=0; a<NumActions(); a++) {
			int next_rob = NextRobPosition(rob_[s], a);

			if (!(a == TagAction() && rob_[s] == opp_[s])) { // No transition upon termination
				for (map<int, double>::const_iterator it = opp_distribution.begin();
					it != opp_distribution.end(); it++) {
					State next;
					next.state_id = RobOppIndicesToStateIndex(next_rob, it->first);
					next.weight = it->second;

					transition_probabilities_[s][a].push_back(next);
				}
			}
		}
	}
}

bool FastTag::Step(State& s, double random_num, int action,
                           double& reward, uint64_t& obs) const {
	FastTagState& state = static_cast<FastTagState&>(s);

	bool terminal = false;
	if (action == TagAction()) {
		if (rob_[state.state_id] == opp_[state.state_id]) {
			reward = TAG_REWARD;
			terminal = true;
		} else {
			reward = -TAG_REWARD;
		}
	} else {
		reward = -1;
	}

	const vector<State>& distribution = transition_probabilities_[state.state_id][action];
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

	return terminal;
}

int FastTag::NumStates() const { return floor_.NumCells() * floor_.NumCells(); }

double FastTag::ObsProb(uint64_t obs, const State& s, int a) const {
	const FastTagState& state = static_cast<const FastTagState&>(s);

	return obs == obs_[state.state_id];
}

const vector<State>& FastTag::TransitionProbability(int s, int a) const {
	return transition_probabilities_[s][a];
}

int FastTag::NextRobPosition(int rob, int a) const {
	Coord pos = floor_.GetCell(rob) + Compass::DIRECTIONS[a];
	if (a != TagAction() && floor_.Inside(pos))
		return floor_.GetIndex(pos);

	return rob;
}

const Floor& FastTag::floor() const { return floor_; }

map<int, double> FastTag::OppTransitionDistribution(int state) const {
	Coord rob = floor_.GetCell(rob_[state]),
				opp = floor_.GetCell(opp_[state]);

  map<int, double> distribution;

	if (opp.x == rob.x) {
		int index = floor_.Inside(opp + Coord(1, 0)) ?
			floor_.GetIndex(opp + Coord(1, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;

		index = floor_.Inside(opp + Coord(-1, 0)) ?
			floor_.GetIndex(opp + Coord(-1, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;
	}  else {
		int dx = opp.x > rob.x ? 1 : -1;
		int index = floor_.Inside(opp + Coord(dx, 0)) ?
			floor_.GetIndex(opp + Coord(dx, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.4;
	}

	if (opp.y == rob.y) {
		int index = floor_.Inside(opp + Coord(0, 1)) ?
			floor_.GetIndex(opp + Coord(0, 1)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;

		index = floor_.Inside(opp + Coord(0, -1)) ?
			floor_.GetIndex(opp + Coord(0, -1)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;
	}  else {
		int dy = opp.y > rob.y ? 1 : -1;
		int index = floor_.Inside(opp + Coord(0, dy)) ?
			floor_.GetIndex(opp + Coord(0, dy)) : floor_.GetIndex(opp);
		distribution[index] += 0.4;
	}

	distribution[floor_.GetIndex(opp)] += 0.2;

  return distribution;
}

void FastTag::PrintTransitions() const {
	for (int s=0; s<NumStates(); s++) {
		// cout << "State " << s << endl;
		for (int a=0; a<NumActions(); a++) {
			// cout << "Applying action " << a << " on " << endl;
			// PrintState(GetState(s));
			for (int i = 0; i < transition_probabilities_[s][a].size();
				i++) {
				const State& next = transition_probabilities_[s][a][i];
				cout << s << "-" << a << "-" << next.state_id << "-"
					<< next.weight << endl;
				PrintState(*GetState(next.state_id));
			}
		}
	}
}

State* FastTag::CreateStartState(string type) const { 
	return start_state_;
}

State* FastTag::RandomStartState() const {
	int n = Random::RANDOM.NextInt(states_.size());
	return new FastTagState(*states_[n]);
}

Belief* FastTag::ExactPrior(const State* s) const {
  vector<State*> particles;
	for (int rob=0; rob<floor_.NumCells(); rob++) {
		for (int opp=0; opp<floor_.NumCells(); opp++) {
			FastTagState* state = static_cast<FastTagState*>(Allocate(RobOppIndicesToStateIndex(rob, opp), 1.0 / floor_.NumCells() / floor_.NumCells()));
			particles.push_back(state);
		}
	}

  ParticleBelief* belief = new ParticleBelief(particles, this);
	belief->state_indexer(this);
	return belief;
}

Belief* FastTag::InitialBelief(const State* start, string type) const {
	Belief* prior = ExactPrior(start);
	return prior;
  /*return new ParticleBelief(
			prior->Sample(Globals::config.n_particles),
			this, prior);*/
}

class FastTagManhattanParticleUpperBound : public ParticleUpperBound {
protected:
	const FastTag* tag_model_;
	vector<int> value_;
public:
	FastTagManhattanParticleUpperBound(const DSPOMDP* model) :
		tag_model_(static_cast<const FastTag*>(model))
	{
		Floor floor = tag_model_->floor_;
		value_.resize(tag_model_->NumStates());
		for (int s=0; s<tag_model_->NumStates(); s++) {
			Coord rob = floor.GetCell(tag_model_->rob_[s]),
					opp = floor.GetCell(tag_model_->opp_[s]);
			int dist = Coord::ManhattanDistance(rob, opp);
			value_[s] = - (1 - Discount(dist)) / (1 - Discount()) + tag_model_->TAG_REWARD * Discount(dist);
		}
	}

	double Value(const State& s) const {
		const FastTagState& state = static_cast<const FastTagState&>(s);
		return value_[state.state_id];
	}
};

class FastTagManhattanBeliefUpperBound : public BeliefUpperBound {
protected:
	const FastTag* tag_model_;
	vector<int> value_;
public:
	FastTagManhattanBeliefUpperBound(const DSPOMDP* model) :
		BeliefUpperBound(),
		tag_model_(static_cast<const FastTag*>(model))
	{
		Floor floor = tag_model_->floor_;
		value_.resize(tag_model_->NumStates());
		for (int s=0; s<tag_model_->NumStates(); s++) {
			Coord rob = floor.GetCell(tag_model_->rob_[s]),
					opp = floor.GetCell(tag_model_->opp_[s]);
			int dist = Coord::ManhattanDistance(rob, opp);
			value_[s] = - (1 - Discount(dist)) / (1 - Discount()) + tag_model_->TAG_REWARD * Discount(dist);
		}
	}

	double Value(const Belief* belief) const {
		const vector<State*>& particles =
			static_cast<const ParticleBelief*>(belief)->particles();

		double value = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			const FastTagState* state = static_cast<const FastTagState*>(particle);
			value += state->weight * value_[state->state_id];
		}
		return value;
	}
};

class FastTagSPParticleUpperBound : public ParticleUpperBound { // Shortest path
protected:
	const FastTag* tag_model_;
	vector<int> value_;
public:
	FastTagSPParticleUpperBound(const DSPOMDP* model) :
		tag_model_(static_cast<const FastTag*>(model))
	{
		Floor floor = tag_model_->floor_;
		value_.resize(tag_model_->NumStates());
		for (int s=0; s<tag_model_->NumStates(); s++) {
			int rob = tag_model_->rob_[s],
					opp = tag_model_->opp_[s];
			int dist = floor.Distance(rob, opp);
			value_[s] = - (1 - Discount(dist)) / (1 - Discount()) + tag_model_->TAG_REWARD * Discount(dist);
		}
	}

	double Value(const State& s) const {
		const FastTagState& state = static_cast<const FastTagState&>(s);

		return value_[state.state_id];
	}
};

ParticleUpperBound* FastTag::CreateParticleUpperBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP") {
		return new MDPUpperBound(this, *this);
	} else if (name == "SP" || name == "DEFAULT") {
		return new FastTagSPParticleUpperBound(this);
	} else if (name == "MANHATTAN") {
		return new FastTagManhattanParticleUpperBound(this);
	} else {
		cerr << "Unsupported particle lower bound: " << name << endl
			<< "Supported types: TRIVIAL, SP, MANHATTAN" << endl
			<< "Default to TRIVIAL" << endl;
		exit(1);
		return NULL;
	}
}

ScenarioUpperBound* FastTag::CreateScenarioUpperBound(string name, 
		string particle_bound_name) const {
	if (name == "TRIVIAL" || name == "DEFAULT" || name == "MDP" ||
			name == "SP" || name == "MANHATTAN") {
		return CreateParticleUpperBound(particle_bound_name);
	} else if(name == "LOOKAHEAD") {
		return new LookaheadUpperBound(this, *this, CreateParticleUpperBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario upper bound: " << name << endl
			<< "Supported types: TRIVIAL" << endl
			<< "Default to TRIVIAL" << endl;
		exit(1);
		return NULL;
	}
}

BeliefUpperBound* FastTag::CreateBeliefUpperBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefUpperBound(this);
	} else if (name == "MANHATTAN") {
		return new FastTagManhattanBeliefUpperBound(this);
	} else {
		cerr << "Unsupported upper bound algorithm: " << name << endl
			<< "Supported types: TRIVIAL" << endl
			<< "Default to TRIVIAL" << endl;
		exit(0);
		return NULL;
	}
}

class FastTagSHRPolicy : public Policy { // Smart History-based Random
private:
	const FastTag tag_model_;
	Floor floor_;

public:
	FastTagSHRPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound),
		tag_model_(static_cast<const FastTag&>(*model)) {
			floor_ = tag_model_.floor();
	}

	int Action(const vector<State*>& particles,
			RandomStreams& streams, History& history) const {
		// If history is empty then we don't know where we are yet
		if (history.Size() == 0) {
			return Random::RANDOM.NextInt(tag_model_.NumActions());
		}

		// If we just saw an opponent then TAG
		if (history.LastObservation() == tag_model_.same_loc_obs_) {
			return tag_model_.TagAction();
		}

		vector<int> actions;
		// Compute rob position
		Coord rob;
		if (tag_model_.same_loc_obs_ != floor_.NumCells()) {
			rob = tag_model_.MostLikelyRobPosition(particles);
		} else {
			rob = floor_.GetCell(history.LastObservation());
		}

		// Don't double back and don't go into walls
		for (int d = 0; d < 4; d++) {
			if (!Compass::Opposite(d, history.LastAction()) && floor_.Inside(rob + Compass::DIRECTIONS[d])) {
				actions.push_back(d);
			}
		}

		if (actions.size() == 0) {
			for (int d = 0; d < 4; d++) {
				if (floor_.Inside(rob + Compass::DIRECTIONS[d]))
					actions.push_back(d);
			}
		}

		if (actions.size() == 0) // Rob may be trapped by the obstacles
			return 0;

		int action = actions[Random::RANDOM.NextInt(actions.size())];
		return action;
	}
};

ScenarioLowerBound* FastTag::CreateScenarioLowerBound(string name, string particle_bound_name) const {
	const DSPOMDP* model = this;
	const StateIndexer* indexer = this;
	const StatePolicy* policy = this;
	const MMAPInferencer* mmap_inferencer = this;
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(model);
	} else if (name == "RANDOM") {
		return new RandomPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "SHR"
		|| (name == "DEFAULT" && same_loc_obs_ != floor_.NumCells())) {
		return new FastTagSHRPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MMAP-MDP") {
		ComputeDefaultActions("MDP");
		return new MMAPStatePolicy(model, *mmap_inferencer, *policy,
				CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MMAP-SP") {
		ComputeDefaultActions("SP");
		return new MMAPStatePolicy(model, *mmap_inferencer, *policy,
				CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MODE-MDP" 
		|| (name == "DEFAULT" && same_loc_obs_ == floor_.NumCells())) {
		ComputeDefaultActions("MDP");
		return new ModeStatePolicy(model, *indexer, *policy,
				CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MODE-SP") {
		ComputeDefaultActions("SP");
		return new ModeStatePolicy(model, *indexer, *policy,
				CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MAJORITY-MDP") {
		// ComputeDefaultActions(Globals::config.default_action);
		ComputeDefaultActions("MDP");
		return new MajorityActionPolicy(model, *policy,
				CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MAJORITY-SP") {
		ComputeDefaultActions("SP");
		return new MajorityActionPolicy(model, *policy,
				CreateParticleLowerBound(particle_bound_name));
	}  else {
		cerr << "Unsupported scenario lower bound: " << name << endl
			<< "Supported types: TRIVIAL, RANDOM, SHR, MMAP-MDP, MMAP-SP, MODE-MDP, MODE-SP, MAJORITY-MDP, MAJORITY-SP" << endl
			<< "Default to TRIVIAL" << endl;
		exit(1);
		return NULL;
	}
}

class TagBlindBeliefPolicy : public BeliefLowerBound {
private:
	vector<vector<double> > alpha_vectors_;
	const FastTag* tag_model_;

public:
	TagBlindBeliefPolicy(const DSPOMDP* model, Belief* belief = NULL) :
		BeliefLowerBound(model, belief),
		tag_model_(static_cast<const FastTag*>(model))
	{
		const_cast<FastTag*>(tag_model_)->ComputeBlindAlpha();
	}

	ValuedAction Value(const Belief* belief) const {
		double bestValue = Globals::NEG_INFTY;
		int bestAction = -1;
		for (int action=0; action<tag_model_->NumActions(); action++) {
			double value = tag_model_->ComputeActionValue(static_cast<const ParticleBelief*>(belief), *tag_model_, action);
			if (value > bestValue) {
				bestValue = value;
				bestAction = action;
			}
		}
		return ValuedAction(bestAction, bestValue);
	}
};

BeliefLowerBound* FastTag::CreateBeliefLowerBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefLowerBound(this);
	} else if (name == "BLIND") {
		return new TagBlindBeliefPolicy(this);
	} else {
		cerr << "Unsupported belief lower bound: " << name << endl
			<< "Supported types: TRIVIAL, BLIND" << endl
			<< "Default to TRIVIAL" << endl;
		exit(1);
		return NULL;
	}
}

void FastTag::PrintState(const State& s, ostream& out) const {
	const FastTagState& state = static_cast<const FastTagState&>(s);

	int aindex = rob_[state.state_id];
	int oindex = opp_[state.state_id];

  for (int y=0; y<floor_.num_rows(); y++) {
		for (int x=0; x<floor_.num_cols(); x++) {
			int index = floor_.GetIndex(x, y);
      if (index == Floor::INVALID)
        out << "#";
      else if (index == aindex && index == oindex)
        out << "Q";
			else if (index == aindex)
				out << "R";
			else if (index == oindex)
				out << "O";
      else
        out << ".";
      //out << " ";
    }
    out << endl;
  }
}

void FastTag::PrintObs(const State& state, uint64_t obs, ostream& out) const {
	if (obs == floor_.NumCells()) {
		out << "On opponent" << endl;
	} else {
		Coord rob = floor_.GetCell(obs);
		out << "Rob at (" << rob.x << ", " << rob.y << ")" << endl;
	}
}

void FastTag::PrintBelief(const Belief& belief, ostream& out) const {
}

void FastTag::PrintAction(int action, ostream& out) const {
	switch(action) {
		case 0: out << "South" << endl; break;
		case 1: out << "East" << endl; break;
		case 2: out << "North" << endl; break;
		case 3: out << "West" << endl; break;
		case 4: out << "Tag" << endl; break;
		default: out << "Wrong action" << endl; exit(1);
	}
}

State* FastTag::Allocate(int state_id, double weight) const {
	FastTagState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* FastTag::Copy(const State* particle) const {
	FastTagState* state = memory_pool_.Allocate();
	*state = *static_cast<const FastTagState*>(particle);
	state->SetAllocated();
	return state;
}

void FastTag::Free(State* particle) const {
	memory_pool_.Free(static_cast<FastTagState*>(particle));
}

int FastTag::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

void FastTag::ComputeDefaultActions(string type) const {
	cerr << "Default action = " << type << endl;
	if (type == "MDP") {
		const_cast<FastTag*>(this)->ComputeOptimalPolicyUsingVI();
		int num_states = NumStates();
		default_action_.resize(num_states);
		for (int s=0; s<num_states; s++)
			default_action_[s] = policy_[s].action;
	} else if (type == "SP") {
		// Follow the shortest path from the robot to the opponent
		default_action_.resize(NumStates());
		for (int s=0; s<NumStates(); s++) {
			default_action_[s] = 0;
			if (rob_[s] == opp_[s]) {
				default_action_[s] = TagAction();
			} else {
				int cur_dist = floor_.Distance(rob_[s], opp_[s]);
				for (int a=0; a<4; a++) {
					int next = NextRobPosition(rob_[s], a);
					int dist = floor_.Distance(next, opp_[s]);

					if (dist < cur_dist) {
						default_action_[s] = a;
						break;
					}
				}
			}
		}
	} else {
		cerr << "Unsupported default action type " << type << endl;
		exit(0);
	}
}

int FastTag::GetAction(const State& state) const {
	return default_action_[GetIndex(&state)];
}

Coord FastTag::MostLikelyOpponentPosition(const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(floor_.NumCells());

	for (int i = 0; i < particles.size(); i++) {
		FastTagState* tagstate = static_cast<FastTagState*>(particles[i]);
		probs[opp_[tagstate->state_id]] += tagstate->weight;
	}

	double maxWeight = 0;
	int opp = -1;
	for (int i=0; i<probs.size(); i++) {
		if (probs[i] > maxWeight) {
			maxWeight = probs[i];
			opp = i;
		}
		probs[i] = 0.0;
	}

	return floor_.GetCell(opp);
}

Coord FastTag::MostLikelyRobPosition(const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(floor_.NumCells());

	double maxWeight = 0;
	int rob = -1;
	for (int i = 0; i < particles.size(); i++) {
		FastTagState* tagstate = static_cast<FastTagState*>(particles[i]);
		int id = rob_[tagstate->state_id];
		probs[id] += tagstate->weight;

		if (probs[id] > maxWeight) {
			maxWeight = probs[id];
			rob = id;
		}
	}

	for (int i=0; i<probs.size(); i++) {
		probs[i] = 0.0;
	}

	return floor_.GetCell(rob);
}

const FastTagState& FastTag::MostLikelyState(const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(NumStates());

	double maxWeight = 0;
	int bestId = -1;
	for (int i = 0; i < particles.size(); i++) {
		FastTagState* tagstate = static_cast<FastTagState*>(particles[i]);
		int id = GetIndex(tagstate);
		probs[id] += tagstate->weight;

		if (probs[id] > maxWeight) {
			maxWeight = probs[id];
			bestId = id;
		}
	}

	for (int i = 0; i < particles.size(); i++) {
		FastTagState* tagstate = static_cast<FastTagState*>(particles[i]);
		probs[GetIndex(tagstate)] = 0;
	}

	return *states_[bestId];
}

const State* FastTag::GetMMAP(const vector<State*>& particles) const {
	Coord rob = MostLikelyRobPosition(particles);
	Coord opp = MostLikelyOpponentPosition(particles);

	int state_id = RobOppIndicesToStateIndex(floor_.GetIndex(rob), floor_.GetIndex(opp));
	return states_[state_id];
}

Belief* FastTag::Tau(const Belief* belief, int action, uint64_t obs) const {
	static vector<double> probs = vector<double>(NumStates());

	const vector<State*>& particles = static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		FastTagState* state = static_cast<FastTagState*>(particles[i]);
		const vector<State>& distribution = transition_probabilities_[GetIndex(state)][action];
		for (int j = 0; j < distribution.size(); j++) {
			const State& next = distribution[j];
			double p = state->weight * next.weight * ObsProb(obs, *(states_[next.state_id]), action);
			probs[next.state_id] += p;
			sum += p;
		}
	}

	vector<State*> new_particles;
	for (int i=0; i<NumStates(); i++) {
		if (probs[i] > 0) {
			State* new_particle = Copy(states_[i]);
			new_particle->weight = probs[i] / sum;
			new_particles.push_back(new_particle);
			probs[i] = 0;
		}
	}

	return new ParticleBelief(new_particles, this);
}

void FastTag::Observe(const Belief* belief, int action, map<uint64_t, double>& obss) const {
	const vector<State*>& particles = static_cast<const ParticleBelief*>(belief)->particles();
	for (int i = 0; i < particles.size(); i++) {
		FastTagState* state = static_cast<FastTagState*>(particles[i]);
		const vector<State>& distribution = transition_probabilities_[GetIndex(state)][action];
		for (int j = 0; j < distribution.size(); j++) {
			const State& next = distribution[j];
			uint64_t obs = obs_[next.state_id];
			double p = state->weight * next.weight;
			obss[obs] += p;
		}
	}
}

double FastTag::StepReward(const Belief* belief, int action) const {
	const vector<State*>& particles = static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		FastTagState* state = static_cast<FastTagState*>(particles[i]);
		double reward = 0;
		if (action == TagAction()) {
			if (rob_[state->state_id] == opp_[state->state_id]) {
				reward = TAG_REWARD;
			} else {
				reward = -TAG_REWARD;
			}
		} else {
			reward = -1;
		}
		sum += state->weight * reward;
	}

	return sum;
}

double FastTag::Reward(int s, int action) const {
	const FastTagState* state = states_[s];
	double reward = 0;
	if (action == TagAction()) {
		if (rob_[state->state_id] == opp_[state->state_id]) {
			reward = TAG_REWARD;
		} else {
			reward = -TAG_REWARD;
		}
	} else {
		reward = -1;
	}
	return reward;
}
