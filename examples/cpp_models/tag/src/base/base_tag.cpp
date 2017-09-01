#include <queue>

#include "base_tag.h"
#include <despot/util/coord.h>
#include <despot/util/floor.h>
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

BaseTag* BaseTag::current_ = NULL;

/* ==============================================================================
 * TagState class
 * ==============================================================================*/

TagState::TagState() {
}

TagState::TagState(int _state_id) {
	state_id = _state_id;
}

string TagState::text() const {
	// NOTE: This will fail if several Tag instances are running
	if (BaseTag::current_ != NULL) {
		int rob = BaseTag::current_->StateIndexToRobIndex(state_id);
		Coord rob_pos = BaseTag::current_->floor_.GetCell(rob);
		int opp = BaseTag::current_->StateIndexToOppIndex(state_id);
		Coord opp_pos = BaseTag::current_->floor_.GetCell(opp);
		return "Rob at " + to_string(rob_pos) + ", Opp at " + to_string(opp_pos);
	} else
		return to_string(state_id);
}

/* ==============================================================================
 * TagBelief class
 * ==============================================================================*/

TagBelief::TagBelief(vector<State*> particles, const BaseTag* model,
	Belief* prior) :
	ParticleBelief(particles, model, prior, false),
	tag_model_(model) {
}

void TagBelief::Update(int action, OBS_TYPE obs) {
	Belief* updated = tag_model_->Tau(this, action, obs);

	for (int i = 0; i < particles_.size(); i++)
		tag_model_->Free(particles_[i]);
	particles_.clear();

	const vector<State*>& new_particles =
		static_cast<ParticleBelief*>(updated)->particles();
	for (int i = 0; i < new_particles.size(); i++)
		particles_.push_back(tag_model_->Copy(new_particles[i]));

	delete updated;
}

/* ==============================================================================
 * TagSHRPolicy class
 * ==============================================================================*/

class TagSHRPolicy: public Policy { // Smart History-based Rollout
private:
	const BaseTag* tag_model_;
	Floor floor_;

public:
	TagSHRPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound),
		tag_model_(static_cast<const BaseTag*>(model)) {
		floor_ = tag_model_->floor();
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		// If history is empty then take a random move
		if (history.Size() == 0) {
			return Random::RANDOM.NextInt(tag_model_->NumActions() - 1);
		}

		// If we just saw an opponent then TAG
		if (history.LastObservation() == tag_model_->same_loc_obs_) {
			return tag_model_->TagAction();
		}

		vector<int> actions;
		// Compute rob position
		Coord rob;
		if (tag_model_->same_loc_obs_ != floor_.NumCells()) {
			rob = tag_model_->MostLikelyRobPosition(particles);
		} else {
			rob = floor_.GetCell(history.LastObservation());
		}

		// Don't double back and don't go into walls
		for (int d = 0; d < 4; d++) {
			if (!Compass::Opposite(d, history.LastAction())
				&& floor_.Inside(rob + Compass::DIRECTIONS[d])) {
				actions.push_back(d);
			}
		}

		// Have to double back
		if (actions.size() == 0) {
			for (int d = 0; d < 4; d++) {
				if (floor_.Inside(rob + Compass::DIRECTIONS[d]))
					actions.push_back(d);
			}
		}

		// Rob may be trapped by the obstacles
		if (actions.size() == 0)
			return 0;

		int action = actions[Random::RANDOM.NextInt(actions.size())];
		return action;
	}
};

/* ==============================================================================
 * TagHistoryModePolicy class
 * ==============================================================================*/

class TagHistoryModePolicy: public Policy {
private:
	const BaseTag* tag_model_;
	Floor floor_;

	int first_action_;
	mutable vector<map<OBS_TYPE, vector<int> > > paths_;
	mutable vector<double> state_probs_;

public:
	TagHistoryModePolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
	Policy(model, bound),
	tag_model_(static_cast<const BaseTag*>(model)) {
		floor_ = tag_model_->floor();
		state_probs_.resize(tag_model_->NumStates());
		paths_.resize(tag_model_->NumActions());
		belief_ = NULL;
	}

	State ComputeMode(Belief* belief) const {
		const vector<State*>& particles = static_cast<ParticleBelief*>(belief)->particles();
		double maxWeight = 0;
		State mode;
		for (int i = 0; particles.size(); i++) {
			State* particle = particles[i];
			int id = particle->state_id;
			state_probs_[id] += particle->weight;

			if (state_probs_[id] > maxWeight) {
				maxWeight = state_probs_[id];
				mode = *particle;
				mode.weight = maxWeight;
			}
		}

		for (int i = 0; i < particles.size(); i++) {
			state_probs_[particles[i]->state_id] = 0;
		}

		return mode;
	}

	void belief(Belief* belief) {
		const vector<State*>& particles = static_cast<ParticleBelief*>(belief)->particles();
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			int id = particle->state_id;
			state_probs_[id] += particle->weight;
		}

		vector<State*> copy;
		for (int s = 0; s < state_probs_.size(); s ++) {
			if (state_probs_[s] > 0.0) {
				copy.push_back(tag_model_->Allocate(s, state_probs_[s]));
				state_probs_[s] = 0.0;
			}
		}

		if (belief_ != NULL)
		delete belief_;

		belief_ = new ParticleBelief(copy, tag_model_, NULL, false);

		State mode = ComputeMode(belief_);

		int rob = tag_model_->StateIndexToRobIndex(mode.state_id);
		int opp = tag_model_->StateIndexToOppIndex(mode.state_id);
		if (rob == opp)
		first_action_ = (mode.weight > 0.99) ? tag_model_->TagAction() : 0;
		else {
			vector<int> policy = floor_.ComputeShortestPath(rob, opp);

			first_action_ = (policy.size() > 0) ? policy[0] : Random::RANDOM.NextInt(4);
		}

		paths_.clear();
	}

	int Action(const vector<State*>& particles,
		RandomStreams& streams, History& history) const {
		if (streams.position() == 0)
		return first_action_;

		if (streams.position() == 1) {
			int action = history.LastAction();
			OBS_TYPE obs = history.LastObservation();

			if (paths_[action].find(obs) == paths_[action].end()) {
				Belief* belief = tag_model_->Tau(belief_, action, obs);

				State mode = ComputeMode(belief);
				int rob = tag_model_->StateIndexToRobIndex(mode.state_id);
				int opp = tag_model_->StateIndexToOppIndex(mode.state_id);
				paths_[action][obs] = floor_.ComputeShortestPath(rob, opp);

				delete belief;
			}
		}

		int action = history.Action(history.Size() - streams.position());
		OBS_TYPE obs = history.Observation(history.Size() - streams.position());
		const vector<int>& policy = paths_[action][obs];

		if (streams.position() - 1 < policy.size()) {
			return policy[streams.position() - 1];
		}

		if (history.LastObservation() == tag_model_->same_loc_obs_)
		return tag_model_->TagAction();

		vector<int> actions;
		// Compute rob position
		Coord rob;
		if (tag_model_->same_loc_obs_ != floor_.NumCells()) {
			rob = tag_model_->MostLikelyRobPosition(particles);
		} else {
			rob = floor_.GetCell(history.LastObservation());
		}

		// Don't double back and don't go into walls
		for (int d = 0; d < 4; d ++) {
			if (!Compass::Opposite(d, history.LastAction()) && floor_.Inside(rob + Compass::DIRECTIONS[d])) {
				actions.push_back(d);
			}
		}

		if (actions.size() == 0) {
			for (int d = 0; d < 4; d ++) {
				if (floor_.Inside(rob + Compass::DIRECTIONS[d]))
				actions.push_back(d);
			}
		}

		if (actions.size() == 0) // Rob may be trapped by the obstacles
		return 0;

		int act = actions[Random::RANDOM.NextInt(actions.size())];
		return act;
	}
};

/* ==============================================================================
 * TagPOMCPPrior class
 * ==============================================================================*/

class TagPOMCPPrior: public POMCPPrior {
private:
	const BaseTag* tag_model_;
public:
	TagPOMCPPrior(const DSPOMDP* model) :
		POMCPPrior(model),
		tag_model_(static_cast<const BaseTag*>(model)) {
	}

	void ComputePreference(const State& state) {
		Coord rob = tag_model_->GetRobPos(&state);

		legal_actions_.clear();
		preferred_actions_.clear();

		for (int a = 0; a < 5; a++) {
			legal_actions_.push_back(a);
		}

		if (history_.Size() != 0) {
			if (history_.LastObservation() == tag_model_->same_loc_obs_) {
				preferred_actions_.push_back(tag_model_->TagAction());
			} else {
				if (tag_model_->robot_pos_unknown_) {
					for (int a = 0; a < 4; a++) {
						if (tag_model_->floor_.Inside(
							rob + Compass::DIRECTIONS[a])) {
							if (!Compass::Opposite(a, history_.LastAction()))
								preferred_actions_.push_back(a);
						}
					}
				}
			}
		}
	}
};

/* ==============================================================================
 * TagManhattanUpperBound class
 * ==============================================================================*/

class TagManhattanUpperBound: public ParticleUpperBound, public BeliefUpperBound {
protected:
	const BaseTag* tag_model_;
	vector<double> value_;
public:
	TagManhattanUpperBound(const BaseTag* model) :
		tag_model_(model) {
		Floor floor = tag_model_->floor_;
		value_.resize(tag_model_->NumStates());
		for (int s = 0; s < tag_model_->NumStates(); s++) {
			Coord rob = floor.GetCell(tag_model_->rob_[s]), opp = floor.GetCell(
				tag_model_->opp_[s]);
			int dist = Coord::ManhattanDistance(rob, opp);
			value_[s] = -(1 - Globals::Discount(dist)) / (1 - Globals::Discount())
				+ tag_model_->TAG_REWARD * Globals::Discount(dist);
		}
	}

	using ParticleUpperBound::Value;
	double Value(const State& s) const {
		const TagState& state = static_cast<const TagState&>(s);
		return value_[state.state_id];
	}

	using BeliefUpperBound::Value;
	double Value(const Belief* belief) const {
		const vector<State*>& particles =
			static_cast<const ParticleBelief*>(belief)->particles();

		double value = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			const TagState* state = static_cast<const TagState*>(particle);
			value += state->weight * value_[state->state_id];
		}
		return value;
	}
};

/* ==============================================================================
 * TagSPParticleUpperBound class
 * ==============================================================================*/

class TagSPParticleUpperBound: public ParticleUpperBound { // Shortest path
protected:
	const BaseTag* tag_model_;
	vector<double> value_;
public:
	TagSPParticleUpperBound(const BaseTag* model) :
		tag_model_(model) {
		Floor floor = tag_model_->floor_;
		value_.resize(tag_model_->NumStates());
		for (int s = 0; s < tag_model_->NumStates(); s++) {
			int rob = tag_model_->rob_[s], opp = tag_model_->opp_[s];
			int dist = (int) floor.Distance(rob, opp);
			value_[s] = -(1 - Globals::Discount(dist)) / (1 - Globals::Discount())
				+ tag_model_->TAG_REWARD * Globals::Discount(dist);
		}
	}

	double Value(const State& s) const {
		const TagState& state = static_cast<const TagState&>(s);

		return value_[state.state_id];
	}
};

/* ==============================================================================
 * TagBlindBeliefPolicy class
 * ==============================================================================*/

class TagBlindBeliefPolicy: public BeliefLowerBound {
private:
	vector<vector<double> > alpha_vectors_;
	const BaseTag* tag_model_;

public:
	TagBlindBeliefPolicy(const BaseTag* model, Belief* belief = NULL) :
		BeliefLowerBound(model, belief),
		tag_model_(model) {
		const_cast<BaseTag*>(tag_model_)->ComputeBlindAlpha();
	}

	ValuedAction Value(const Belief* belief) const {
		double bestValue = Globals::NEG_INFTY;
		int bestAction = -1;
		for (int action = 0; action < tag_model_->NumActions(); action++) {
			double value = tag_model_->ComputeActionValue(
				static_cast<const ParticleBelief*>(belief), *tag_model_,
				action);
			if (value > bestValue) {
				bestValue = value;
				bestAction = action;
			}
		}

		return ValuedAction(bestAction, bestValue);
	}
};

/* ==============================================================================
 * BaseTag class
 * ==============================================================================*/
double BaseTag::TAG_REWARD = 10;

string BaseTag::RandomMap(int height, int width, int obstacles) {
	string map(height * (width + 1) - 1, '.');
	for (int h = 1; h < height; h++)
		map[h * (width + 1) - 1] = '\n';

	for (int i = 0; i < obstacles;) {
		int p = Random::RANDOM.NextInt(map.length());
		if (map[p] != '\n' && map[p] != '#') {
			map[p] = '#';
			i++;
		}
	}

	return "mapSize = " + to_string(height) + " " + to_string(width) + "\n" + to_string(map);
}

void BaseTag::ReadConfig(istream& is) {
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
				}
			}

			floor_.ComputeDistances();
		} else if (key == "width-height-obstacles") {
			int h, w, o;
			is >> h >> w >> o;
			istringstream iss(RandomMap(h, w, o));
			ReadConfig(iss);
		}
	}
}

BaseTag::BaseTag() :
  robot_pos_unknown_(false) {
	current_ = this;
	istringstream iss(
		string("mapSize = 5 10\n") + string("#####...##\n")
			+ string("#####...##\n") + string("#####...##\n")
			+ string("...........\n") + string("..........."));
	Init(iss);
}

BaseTag::BaseTag(string params_file) :
  robot_pos_unknown_(false) {
	current_ = this;
	ifstream fin(params_file.c_str(), ifstream::in);
	Init(fin);
}

void BaseTag::Init(istream& is) {
	ReadConfig(is);

	TagState* state;
	states_.resize(NumStates());
	rob_.resize(NumStates());
	opp_.resize(NumStates());
	for (int rob = 0; rob < floor_.NumCells(); rob++) {
		for (int opp = 0; opp < floor_.NumCells(); opp++) {
			int s = RobOppIndicesToStateIndex(rob, opp);
			state = new TagState(s);
			states_[s] = state;
			rob_[s] = rob;
			opp_[s] = opp;
		}
	}

	// Build transition matrix
	transition_probabilities_.resize(NumStates());
	for (int s = 0; s < NumStates(); s++) {
		transition_probabilities_[s].resize(NumActions());

		const map<int, double>& opp_distribution = OppTransitionDistribution(s);
		for (int a = 0; a < NumActions(); a++) {
			transition_probabilities_[s][a].clear();

			int next_rob = NextRobPosition(rob_[s], a);

			if (!(a == TagAction() && rob_[s] == opp_[s])) { // No transition upon termination
				for (map<int, double>::const_iterator it = opp_distribution.begin();
					it != opp_distribution.end(); it++) {
					State next;
					next.state_id = RobOppIndicesToStateIndex(next_rob,
						it->first);
					next.weight = it->second;

					transition_probabilities_[s][a].push_back(next);
				}
			}
		}
	}
}

BaseTag::~BaseTag() {
}

bool BaseTag::Step(State& s, double random_num, int action,
	double& reward) const {
	TagState& state = static_cast<TagState&>(s);

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

	return terminal;
}

int BaseTag::NumStates() const {
	return floor_.NumCells() * floor_.NumCells();
}

const vector<State>& BaseTag::TransitionProbability(int s, int a) const {
	return transition_probabilities_[s][a];
}

int BaseTag::NextRobPosition(int rob, int a) const {
	Coord pos = floor_.GetCell(rob) + Compass::DIRECTIONS[a];
	if (a != TagAction() && floor_.Inside(pos))
		return floor_.GetIndex(pos);

	return rob;
}

const Floor& BaseTag::floor() const {
	return floor_;
}

map<int, double> BaseTag::OppTransitionDistribution(int state) const {
	Coord rob = floor_.GetCell(rob_[state]), opp = floor_.GetCell(opp_[state]);

	map<int, double> distribution;

	if (opp.x == rob.x) {
		int index =
			floor_.Inside(opp + Coord(1, 0)) ?
				floor_.GetIndex(opp + Coord(1, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;

		index =
			floor_.Inside(opp + Coord(-1, 0)) ?
				floor_.GetIndex(opp + Coord(-1, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;
	} else {
		int dx = opp.x > rob.x ? 1 : -1;
		int index =
			floor_.Inside(opp + Coord(dx, 0)) ?
				floor_.GetIndex(opp + Coord(dx, 0)) : floor_.GetIndex(opp);
		distribution[index] += 0.4;
	}

	if (opp.y == rob.y) {
		int index =
			floor_.Inside(opp + Coord(0, 1)) ?
				floor_.GetIndex(opp + Coord(0, 1)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;

		index =
			floor_.Inside(opp + Coord(0, -1)) ?
				floor_.GetIndex(opp + Coord(0, -1)) : floor_.GetIndex(opp);
		distribution[index] += 0.2;
	} else {
		int dy = opp.y > rob.y ? 1 : -1;
		int index =
			floor_.Inside(opp + Coord(0, dy)) ?
				floor_.GetIndex(opp + Coord(0, dy)) : floor_.GetIndex(opp);
		distribution[index] += 0.4;
	}

	distribution[floor_.GetIndex(opp)] += 0.2;

	return distribution;
}

void BaseTag::PrintTransitions() const {
	for (int s = 0; s < NumStates(); s++) {
		cout << "State " << s << endl;
		PrintState(*GetState(s));
		for (int a = 0; a < NumActions(); a++) {
			cout << "Applying action " << a << " on " << endl;
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

State* BaseTag::CreateStartState(string type) const {
	int n = Random::RANDOM.NextInt(states_.size());
	return new TagState(*states_[n]);
}

ParticleUpperBound* BaseTag::CreateParticleUpperBound(string name) const {
	if (name == "TRIVIAL") {
		return  new TrivialParticleUpperBound(this);
	} else if (name == "MDP") {
		return new MDPUpperBound(this, *this);
	} else if (name == "SP" || name == "DEFAULT") {
		return new TagSPParticleUpperBound(this);
	} else if (name == "MANHATTAN") {
		return new TagManhattanUpperBound(this);
	} else {
		if (name != "print")
			cerr << "Unsupported particle lower bound: " << name << endl;
		cerr << "Supported types: TRIVIAL, MDP, SP, MANHATTAN (default to SP)" << endl;
		exit(1);
		return NULL;
	}
}

ScenarioUpperBound* BaseTag::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL" || name == "DEFAULT" || name == "MDP" ||
			name == "SP" || name == "MANHATTAN") {
		return CreateParticleUpperBound(name);
	} else if (name == "LOOKAHEAD") {
		return new LookaheadUpperBound(this, *this,
			CreateParticleUpperBound(particle_bound_name));
	} else {
		if (name != "print")
			cerr << "Unsupported upper bound: " << name << endl;
		cerr << "Supported types: TRIVIAL, MDP, SP, MANHATTAN, LOOKAHEAD (default to SP)" << endl;
		cerr << "With base upper bound: LOOKAHEAD" << endl;
		exit(1);
		return NULL;
	}
}

BeliefUpperBound* BaseTag::CreateBeliefUpperBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefUpperBound(this);
	} else if (name == "DEFAULT" || name == "MDP") {
		return new MDPUpperBound(this, *this);
	} else if (name == "MANHATTAN") {
		return new TagManhattanUpperBound(this);
	} else {
		if (name != "print")
			cerr << "Unsupported belief upper bound: " << name << endl;
		cerr << "Supported types: TRIVIAL, MDP, MANHATTAN (default to MDP)" << endl;
		exit(1);
		return NULL;
	}
}

ScenarioLowerBound* BaseTag::CreateScenarioLowerBound(string name, string
	particle_bound_name) const {
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
		return new TagSHRPolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "HM") {
		return new TagHistoryModePolicy(model,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MMAP-MDP") {
		ComputeDefaultActions("MDP");
		return new MMAPStatePolicy(model, *mmap_inferencer,
			*policy, CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MMAP-SP") {
		ComputeDefaultActions("SP");
		return new MMAPStatePolicy(model, *mmap_inferencer,
			*policy, CreateParticleLowerBound(particle_bound_name));
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
		ComputeDefaultActions("MDP");
		return new MajorityActionPolicy(model, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MAJORITY-SP") {
		ComputeDefaultActions("SP");
		return new MajorityActionPolicy(model, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		if (name != "print")
			cerr << "Unsupported lower bound: " << name << endl;
		cerr << "Supported types: TRIVIAL, RANDOM, SHR, MODE-MDP, MODE-SP, MAJORITY-MDP, MAJORITY-SP (default to MODE-MDP)" << endl;
		cerr << "With base lower bound: except TRIVIAL" << endl;
		exit(1);
		return NULL;
	}
}

BeliefLowerBound* BaseTag::CreateBeliefLowerBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefLowerBound(this);
	} else if (name == "DEFAULT" || name == "BLIND") {
		return new TagBlindBeliefPolicy(this);
	} else {
		cerr << "Unsupported belief lower bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

POMCPPrior* BaseTag::CreatePOMCPPrior(string name) const {
	if (name == "UNIFORM") {
		return new UniformPOMCPPrior(this);
	} else if (name == "DEFAULT" || name == "SHR") {
		return new TagPOMCPPrior(this);
	} else {
		cerr << "Unsupported POMCP prior: " << name << endl;
		exit(1);
		return NULL;
	}
}

void BaseTag::PrintState(const State& s, ostream& out) const {
	const TagState& state = static_cast<const TagState&>(s);

	int aindex = rob_[state.state_id];
	int oindex = opp_[state.state_id];

	for (int y = floor_.num_rows()-1; y >= 0; y--) {
		for (int x = 0; x < floor_.num_cols(); x++) {
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
		}
		out << endl;
	}
}

void BaseTag::PrintBelief(const Belief& belief, ostream& out) const {
}

void BaseTag::PrintAction(int action, ostream& out) const {
	switch(action) {
		case 0: out << "North" << endl; break;
		case 1: out << "East" << endl; break;
		case 2: out << "South" << endl; break;
		case 3: out << "West" << endl; break;
		case 4: out << "Tag" << endl; break;
		default: out << "Wrong action" << endl; exit(1);
	}
}

State* BaseTag::Allocate(int state_id, double weight) const {
	TagState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* BaseTag::Copy(const State* particle) const {
	TagState* state = memory_pool_.Allocate();
	*state = *static_cast<const TagState*>(particle);
	state->SetAllocated();
	return state;
}

void BaseTag::Free(State* particle) const {
	memory_pool_.Free(static_cast<TagState*>(particle));
}

int BaseTag::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

void BaseTag::ComputeDefaultActions(string type) const {
	if (type == "MDP") {
		const_cast<BaseTag*>(this)->ComputeOptimalPolicyUsingVI();
		int num_states = NumStates();
		default_action_.resize(num_states);

		for (int s = 0; s < num_states; s++) {
			default_action_[s] = policy_[s].action;
		}
	} else if (type == "SP") {
		// Follow the shortest path from the robot to the opponent
		default_action_.resize(NumStates());
		for (int s = 0; s < NumStates(); s++) {
			default_action_[s] = 0;
			if (rob_[s] == opp_[s]) {
				default_action_[s] = TagAction();
			} else {
				double cur_dist = floor_.Distance(rob_[s], opp_[s]);
				for (int a = 0; a < 4; a++) {
					int next = NextRobPosition(rob_[s], a);
					double dist = floor_.Distance(next, opp_[s]);

					if (dist < cur_dist) {
						default_action_[s] = a;
						break;
					}
				}
			}
		}
	} else {
		cerr << "Unsupported default action type " << type << endl;
		exit(1);
	}
}

int BaseTag::GetAction(const State& state) const {
	return default_action_[GetIndex(&state)];
}

Coord BaseTag::MostLikelyOpponentPosition(
	const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(floor_.NumCells());

	for (int i = 0; i < particles.size(); i++) {
		TagState* tagstate = static_cast<TagState*>(particles[i]);
		probs[opp_[tagstate->state_id]] += tagstate->weight;
	}

	double maxWeight = 0;
	int opp = -1;
	for (int i = 0; i < probs.size(); i++) {
		if (probs[i] > maxWeight) {
			maxWeight = probs[i];
			opp = i;
		}
		probs[i] = 0.0;
	}

	return floor_.GetCell(opp);
}

Coord BaseTag::MostLikelyRobPosition(const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(floor_.NumCells());

	double maxWeight = 0;
	int rob = -1;
	for (int i = 0; i < particles.size(); i++) {
		TagState* tagstate = static_cast<TagState*>(particles[i]);
		int id = rob_[tagstate->state_id];
		probs[id] += tagstate->weight;

		if (probs[id] > maxWeight) {
			maxWeight = probs[id];
			rob = id;
		}
	}

	for (int i = 0; i < probs.size(); i++) {
		probs[i] = 0.0;
	}

	return floor_.GetCell(rob);
}

const TagState& BaseTag::MostLikelyState(
	const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(NumStates());

	double maxWeight = 0;
	int bestId = -1;
	for (int i = 0; i < particles.size(); i++) {
		TagState* tagstate = static_cast<TagState*>(particles[i]);
		int id = GetIndex(tagstate);
		probs[id] += tagstate->weight;

		if (probs[id] > maxWeight) {
			maxWeight = probs[id];
			bestId = id;
		}
	}

	for (int i = 0; i < particles.size(); i++) {
		TagState* tagstate = static_cast<TagState*>(particles[i]);
		probs[GetIndex(tagstate)] = 0;
	}

	return *states_[bestId];
}

const State* BaseTag::GetMMAP(const vector<State*>& particles) const {
	Coord rob = MostLikelyRobPosition(particles);
	Coord opp = MostLikelyOpponentPosition(particles);

	int state_id = RobOppIndicesToStateIndex(floor_.GetIndex(rob),
		floor_.GetIndex(opp));
	return states_[state_id];
}

Belief* BaseTag::Tau(const Belief* belief, int action, OBS_TYPE obs) const {
	static vector<double> probs = vector<double>(NumStates());

	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		TagState* state = static_cast<TagState*>(particles[i]);
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

double BaseTag::StepReward(const Belief* belief, int action) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		TagState* state = static_cast<TagState*>(particle);
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

double BaseTag::Reward(int s, int action) const {
	const TagState* state = states_[s];
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

} // namespace despot
