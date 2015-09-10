#include "simple_rock_sample.h"

using namespace std;

/* =============================================================================
 * SimpleState class
 * =============================================================================*/

SimpleState::SimpleState() {
}

SimpleState::SimpleState(int _rover_position, int _rock_status) {
	rover_position = _rover_position;
	rock_status = _rock_status;
}

SimpleState::~SimpleState() {
}

string SimpleState::text() const {
	return "rover position = " + to_string(rover_position) + " rock_status = " +
		to_string(rock_status);
}

/* =============================================================================
 * SimpleRockSample class
 * =============================================================================*/

SimpleRockSample::SimpleRockSample() {
}

/* ======
 * Action
 * ======*/

int SimpleRockSample::NumActions() const {
	return 4;
}

/* ==============================
 * Deterministic simulative model
 * ==============================*/

bool SimpleRockSample::Step(State& state, double rand_num, int action,
	double& reward, OBS_TYPE& obs) const {
	SimpleState& simple_state = static_cast<SimpleState&>(state);
	int& rover_position = simple_state.rover_position;
	int& rock_status = simple_state.rock_status;

	obs = 1; // default observation
	if (rover_position == 0) {
		if (action == A_SAMPLE) {
			reward = rock_status ? 10 : -10;
			rock_status = 0;
		} else if (action == A_CHECK) {
			reward = 0;
			obs = rock_status;
		} else if (action == A_WEST) {
			reward = -100;
			rover_position = 2;
		} else {
			reward = 0;
			rover_position = 1;
		}
	} else if (rover_position == 1) {
		if (action == A_SAMPLE) {
			reward = -100;
			rover_position = 2;
		} else if (action == A_CHECK) {
			reward = 0;
			obs = (rand_num > 0.20) ? rock_status : (1 - rock_status);
		} else if (action == A_WEST) {
			reward = 0;
			rover_position = 0;
		} else {
			reward = 10;
			rover_position = 2;
		}
	} else {
		reward = 0;
	}

	return rover_position == 2;
}

/* ================================================
 * Functions related to beliefs and starting states
 * ================================================*/

double SimpleRockSample::ObsProb(OBS_TYPE obs, const State& state,
	int action) const {
	if (action == A_CHECK) {
		const SimpleState& simple_state = static_cast<const SimpleState&>(state);
		int rover_position = simple_state.rover_position;
		int rock_status = simple_state.rock_status;

		if (rover_position == 0) {
			return obs == rock_status;
		} else if (rover_position == 1) {
			return (obs == rock_status) ? 0.8 : 0.2;
		}
	}

	return obs == 1;
}

State* SimpleRockSample::CreateStartState(string type) const {
	return new SimpleState(1, Random::RANDOM.NextInt(2));
}

Belief* SimpleRockSample::InitialBelief(const State* start, string type) const {
	if (type == "DEFAULT" || type == "PARTICLE") {
		vector<State*> particles;

		SimpleState* good_rock = static_cast<SimpleState*>(Allocate(-1, 0.5));
		good_rock->rover_position = 1;
		good_rock->rock_status = 1;
		particles.push_back(good_rock);

		SimpleState* bad_rock = static_cast<SimpleState*>(Allocate(-1, 0.5));
		bad_rock->rover_position = 1;
		bad_rock->rock_status = 0;
		particles.push_back(bad_rock);

		return new ParticleBelief(particles, this);
	} else {
		cerr << "[SimpleRockSample::InitialBelief] Unsupported belief type: " << type << endl;
		exit(1);
	}
}

/* ========================
 * Bound-related functions.
 * ========================*/

double SimpleRockSample::GetMaxReward() const {
	return 10;
}

class SimpleRockSampleParticleUpperBound: public ParticleUpperBound {
protected:
	// upper_bounds_[pos][status]:
	//   max possible reward when rover_position = pos, and rock_status = status.
	vector<vector<double> > upper_bounds_;

public:
	SimpleRockSampleParticleUpperBound(const DSPOMDP* model) {
		upper_bounds_.resize(3);
		upper_bounds_[0].push_back(Discount(1) * 10);
		upper_bounds_[0].push_back(10 + Discount(2) * 10);
		upper_bounds_[1].push_back(10);
		upper_bounds_[1].push_back(Discount(1) * 10 + Discount(3) * 10);
		if (upper_bounds_[1][1] < 10)
			upper_bounds_[1][1] = 10;
		upper_bounds_[2].push_back(0);
		upper_bounds_[2].push_back(0);
	}

	double Value(const State& s) const {
		const SimpleState& state = static_cast<const SimpleState&>(s);
		return upper_bounds_[state.rover_position][state.rock_status];
	}
};

ScenarioUpperBound* SimpleRockSample::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	ScenarioUpperBound* bound = NULL;
	if (name == "TRIVIAL" || name == "DEFAULT") {
		bound = new TrivialParticleUpperBound(this);
	} else if (name == "MAX") {
		bound = new SimpleRockSampleParticleUpperBound(this);
	} else {
		cerr << "Unsupported base upper bound: " << name << endl;
		exit(0);
	}
	return bound;
}

ValuedAction SimpleRockSample::GetMinRewardAction() const {
	return ValuedAction(A_EAST, 0);
}

class SimpleRockSampleEastPolicy: public Policy {
public:
	SimpleRockSampleEastPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound) {
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		return 1; // move east
	}
};

ScenarioLowerBound* SimpleRockSample::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	ScenarioLowerBound* bound = NULL;
	if (name == "TRIVIAL" || name == "DEFAULT") {
		bound = new TrivialParticleLowerBound(this);
	} else if (name == "EAST") {
		bound = new SimpleRockSampleEastPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported lower bound algorithm: " << name << endl;
		exit(0);
	}
	return bound;
}

/* =================
 * Memory management
 * =================*/

State* SimpleRockSample::Allocate(int state_id, double weight) const {
	SimpleState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* SimpleRockSample::Copy(const State* particle) const {
	SimpleState* state = memory_pool_.Allocate();
	*state = *static_cast<const SimpleState*>(particle);
	state->SetAllocated();
	return state;
}

void SimpleRockSample::Free(State* particle) const {
	memory_pool_.Free(static_cast<SimpleState*>(particle));
}

int SimpleRockSample::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

/* =======
 * Display
 * =======*/

void SimpleRockSample::PrintState(const State& state, ostream& out) const {
	const SimpleState& simple_state = static_cast<const SimpleState&>(state);

	out << "Rover = " << simple_state.rover_position << "; Rock = "
		<< (simple_state.rock_status ? "GOOD" : "BAD") << endl;
}

void SimpleRockSample::PrintObs(const State& state, OBS_TYPE observation,
	ostream& out) const {
	out << (observation ? "GOOD" : "BAD") << endl;
}

void SimpleRockSample::PrintBelief(const Belief& belief, ostream& out) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief&>(belief).particles();

	double rock_status = 0;
	vector<double> pos_probs(3);
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		const SimpleState* state = static_cast<const SimpleState*>(particle);
		rock_status += state->rock_status * particle->weight;
		pos_probs[state->rover_position] += particle->weight;
	}

	out << "Rock belief: " << rock_status << endl;

	out << "Position belief:" << " LEFT" << ":" << pos_probs[0] << " MIDDLE"
		<< ":" << pos_probs[1] << " RIGHT" << ":" << pos_probs[2] << endl;
}

void SimpleRockSample::PrintAction(int action, ostream& out) const {
	if (action == A_SAMPLE)
		out << "Sample" << endl;
	if (action == A_CHECK)
		out << "Check" << endl;
	if (action == A_EAST)
		out << "EAST " << endl;
	if (action == A_WEST)
		out << "West" << endl;
}
