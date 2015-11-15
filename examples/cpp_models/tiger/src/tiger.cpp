#include "tiger.h"

using namespace std;

namespace despot {

const int Tiger::LEFT = 0;
const int Tiger::RIGHT = 1;
const int Tiger::LISTEN = 2;
const double Tiger::NOISE = 0.15;

/* =============================================================================
 * TigerState class
 * =============================================================================*/

TigerState::TigerState() :
	tiger_position(0) {
}

TigerState::TigerState(int position) :
	tiger_position(position) {
}

string TigerState::text() const {
	return tiger_position == Tiger::LEFT ? "LEFT" : "RIGHT";
}

/* =============================================================================
 * OptimalTigerPolicy class
 * =============================================================================*/

class OptimalTigerPolicy: public Policy {
public:
	OptimalTigerPolicy(const DSPOMDP* model,
		ParticleLowerBound* bound) :
		Policy(model, bound) {
	}

	// NOTE: optimal for noise = 0.15
	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		/*
		 if (history.Size() == 0 || history.LastAction() != LISTEN) {
		 actions->push_back(LISTEN);
		 return actions;
		 }

		 actions->push_back(history.LastObservation());
		 */

		int count_diff = 0;
		for (int i = history.Size() - 1;
			i >= 0 && history.Action(i) == Tiger::LISTEN; i--)
			count_diff += history.Observation(i) == Tiger::LEFT ? 1 : -1;

		if (count_diff >= 2)
			return Tiger::RIGHT;
		else if (count_diff <= -2)
			return Tiger::LEFT;
		else
			return Tiger::LISTEN;
	}
};

/* =============================================================================
 * Tiger class
 * =============================================================================*/

Tiger::Tiger() {
}

bool Tiger::Step(State& s, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	TigerState& state = static_cast<TigerState&>(s);
	bool terminal = false;

	if (action == LEFT || action == RIGHT) {
		reward = state.tiger_position != action ? 10 : -100;
		state.tiger_position = random_num <= 0.5 ? LEFT : RIGHT;
		obs = 2; // can use arbitary observation
	} else {
		reward = -1;
		if (random_num <= 1 - NOISE)
			obs = state.tiger_position;
		else
			obs = (LEFT + RIGHT - state.tiger_position);
	}
	return terminal;
}

int Tiger::NumStates() const {
	return 2;
}

int Tiger::NumActions() const {
	return 3;
}

double Tiger::ObsProb(OBS_TYPE obs, const State& s, int a) const {
	const TigerState& state = static_cast<const TigerState&>(s);

	if (a != LISTEN)
		return obs == 2;

	return state.tiger_position == obs ? (1 - NOISE) : NOISE;
}

State* Tiger::CreateStartState(string type) const {
	return new TigerState(Random::RANDOM.NextInt(2));
}

Belief* Tiger::InitialBelief(const State* start, string type) const {
	vector<State*> particles;
	TigerState* left = static_cast<TigerState*>(Allocate(-1, 0.5));
	left->tiger_position = LEFT;
	particles.push_back(left);
	TigerState* right = static_cast<TigerState*>(Allocate(-1, 0.5));
	right->tiger_position = RIGHT;
	particles.push_back(right);
	return new ParticleBelief(particles, this);
}

ScenarioLowerBound* Tiger::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	ScenarioLowerBound* bound = NULL;
	if (name == "TRIVIAL" || name == "DEFAULT") {
		bound = new TrivialParticleLowerBound(this);
	} else if (name == "RANDOM") {
		bound = new RandomPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "LEFT") {
		bound = new BlindPolicy(this, LEFT,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "RIGHT") {
		bound = new BlindPolicy(this, RIGHT,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "LISTEN") {
		bound = new BlindPolicy(this, LISTEN,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "OPTIMAL") {
		bound = new OptimalTigerPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario lower bound: " << name << endl;
		exit(1);
	}
	return bound;
}

void Tiger::PrintState(const State& state, ostream& out) const {
	const TigerState& tigerstate = static_cast<const TigerState&>(state);
	out << tigerstate.text() << endl;
}

void Tiger::PrintBelief(const Belief& belief, ostream& out) const {
}

void Tiger::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	out << (obs == LEFT ? "LEFT" : "RIGHT") << endl;
}

void Tiger::PrintAction(int action, ostream& out) const {
	if (action == LEFT) {
		out << "Open left" << endl;
	} else if (action == RIGHT) {
		out << "Open right" << endl;
	} else {
		out << "Listen" << endl;
	}
}

State* Tiger::Allocate(int state_id, double weight) const {
	TigerState* particle = memory_pool_.Allocate();
	particle->state_id = state_id;
	particle->weight = weight;
	return particle;
}

State* Tiger::Copy(const State* particle) const {
	TigerState* new_particle = memory_pool_.Allocate();
	*new_particle = *static_cast<const TigerState*>(particle);
	new_particle->SetAllocated();
	return new_particle;
}

void Tiger::Free(State* particle) const {
	memory_pool_.Free(static_cast<TigerState*>(particle));
}

int Tiger::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

} // namespace despot
