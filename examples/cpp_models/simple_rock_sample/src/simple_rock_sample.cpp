#include "simple_rock_sample.h"

using namespace std;

namespace despot {

/* =============================================================================
 * SimpleState class
 * =============================================================================*/

SimpleState::SimpleState() {
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
    SimpleState& simple_state = static_cast < SimpleState& >(state);
    int& rover_position = simple_state.rover_position;
    int& rock_status = simple_state.rock_status;
 
    if (rover_position == LEFT) {
        if (action == A_SAMPLE) {
            reward = (rock_status == R_GOOD) ? 10 : -10;
            obs = O_GOOD;
            rock_status = R_BAD;
        } else if (action == A_CHECK) {
            reward = 0;
            // when the rover at LEFT, its observation is correct with probability 1
            obs = (rock_status == R_GOOD) ? O_GOOD : O_BAD;  
        } else if (action == A_WEST) {
            reward = -100;
            // moving does not incur observation, setting a default observation 
            // note that we can also set the default observation to O_BAD, as long
            // as it is consistent.
            obs = O_GOOD;
            return true; // Moving off the grid terminates the task. 
        } else { // moving EAST
            reward = 0;
            // moving does not incur observation, setting a default observation
            obs = O_GOOD;
            rover_position = MIDDLE;
        }
    } else if (rover_position  == MIDDLE) {
        if (action == A_SAMPLE) {
            reward = -100;
            // moving does not incur observation, setting a default observation 
            obs = O_GOOD;
            return true; // sampling in the grid where there is no rock terminates the task
        } else if (action == A_CHECK) {
            reward = 0;
            // when the rover is at MIDDLE, its observation is correct with probability 0.8
            obs =  (rand_num > 0.20) ? rock_status : (1 - rock_status);
        } else if (action == A_WEST) {
            reward = 0;
            // moving does not incur observation, setting a default observation 
            obs = O_GOOD;
            rover_position = LEFT;
        } else { //moving EAST to exit
            reward = 10;
            obs = O_GOOD;
            rover_position = RIGHT;
        }
    }

 	if(rover_position == RIGHT) return true;
 	else return false;
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

		if (rover_position == LEFT) {
			// when the rover at LEFT, its observation is correct with probability 1
			return obs == rock_status;
		} else if (rover_position == MIDDLE) {
			// when the rover at MIDDLE, its observation is correct with probability 0.8
			return (obs == rock_status) ? 0.8 : 0.2;
		}
	}

	// when the actions are not A_CHECK, the rover does not receive any observations.
	// Assume it receives a default observation with probability 1.
	return obs == O_GOOD;
}

State* SimpleRockSample::CreateStartState(string type) const {
	return new SimpleState(1, Random::RANDOM.NextInt(2));
}

Belief* SimpleRockSample::InitialBelief(const State* start, string type) const {
	if (type == "DEFAULT" || type == "PARTICLE") {
		vector<State*> particles;

		SimpleState* good_rock = static_cast<SimpleState*>(Allocate(-1, 0.5));
		good_rock->rover_position = MIDDLE;
		good_rock->rock_status = O_GOOD;
		particles.push_back(good_rock);

		SimpleState* bad_rock = static_cast<SimpleState*>(Allocate(-1, 0.5));
		bad_rock->rover_position = MIDDLE;
		bad_rock->rock_status = O_BAD;
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
/*
Note: in the following bound-related functions, only GetMaxReward() and 
GetMinRewardAction() functions are required to be implemented. The other 
functions (or classes) are for custom bounds. You don't need to write them
if you don't want to use your own custom bounds. However, it is highly 
recommended that you build the bounds based on the domain knowledge because
it often improves the performance. Read the tutorial for more details on how
to implement custom bounds.
*/
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
		upper_bounds_[0].push_back(Globals::Discount(1) * 10);
		upper_bounds_[0].push_back(10 + Globals::Discount(2) * 10);
		upper_bounds_[1].push_back(10);
		upper_bounds_[1].push_back(Globals::Discount(1) * 10 + Globals::Discount(3) * 10);
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
	enum { // action
		A_SAMPLE = 0, A_EAST = 1, A_WEST = 2, A_CHECK = 3
	};
	SimpleRockSampleEastPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound) {
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		return A_EAST; // move east
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

} // namespace despot
