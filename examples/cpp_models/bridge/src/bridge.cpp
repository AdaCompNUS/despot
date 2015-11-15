#include "bridge.h"
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * BridgeState class
 * =============================================================================*/

BridgeState::BridgeState() :
	position(0) {
}

BridgeState::BridgeState(int _position) :
	position(_position) {
}

string BridgeState::text() const {
	return "Man at " + to_string(position);
}

/* =============================================================================
 * Bridge class
 * =============================================================================*/

Bridge::Bridge() {
}

int Bridge::LEFT = 0;
int Bridge::RIGHT = 1;
int Bridge::HELP = 2;
int Bridge::BRIDGELENGTH = 10;

bool Bridge::Step(State& s, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	BridgeState& state = static_cast<BridgeState&>(s);
	bool terminal = false;
	int& position = state.position;

	obs = 1;
	if (action == LEFT) {
		reward = -1;
		if (position > 0)
			position--;
	} else if (action == RIGHT) {
		if (position < BRIDGELENGTH - 1) {
			reward = -1;
			position++;
		} else {
			reward = 0;
			terminal = true;
		}
	} else { // HELP
		reward = -20 - position;
		terminal = true;
	}
	return terminal;
}

int Bridge::NumStates() const {
	return BRIDGELENGTH;
}

int Bridge::NumActions() const {
	return 3;
}

double Bridge::ObsProb(OBS_TYPE obs, const State& s, int a) const {
	return obs == 1;
}

State* Bridge::CreateStartState(string type) const {
	// return new BridgeState(Random::RANDOM.NextInt(2));
	return new BridgeState(0);	// Always start at the left end
}

Belief* Bridge::InitialBelief(const State* start, string type) const {
	vector<State*> particles;
	for (int pos = 0; pos <= 1; pos++) {
		for (int i = 0; i < 100; i++) {
			BridgeState* state = static_cast<BridgeState*>(Allocate(pos, 0.005));
			state->position = pos;
			particles.push_back(state);
		}
	}

	return new ParticleBelief(particles, this);
}

void Bridge::PrintState(const State& state, ostream& out) const {
	out << state.text() << endl;
}

void Bridge::PrintBelief(const Belief& belief, ostream& out) const {
}

void Bridge::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	out << obs << endl;
}
;

void Bridge::PrintAction(int action, ostream& out) const {
	if (action == LEFT) {
		cout << "Move left" << endl;
	} else if (action == RIGHT) {
		cout << "Move right" << endl;
	} else {
		cout << "Call for help" << endl;
	}
}

ScenarioLowerBound* Bridge::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(this);
	} else if (name == "RANDOM") {
		return new RandomPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "CALL_FOR_HELP" || name == "DEFAULT") {
		return new BlindPolicy(this, HELP,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported lower bound algorithm: " << name << endl;
		exit(1);
	}
}

State* Bridge::Allocate(int state_id, double weight) const {
	BridgeState* particle = memory_pool_.Allocate();
	particle->state_id = state_id;
	particle->weight = weight;
	return particle;
}

State* Bridge::Copy(const State* particle) const {
	BridgeState* new_particle = memory_pool_.Allocate();
	*new_particle = *static_cast<const BridgeState*>(particle);
	new_particle->SetAllocated();
	return new_particle;
}

void Bridge::Free(State* particle) const {
	memory_pool_.Free(static_cast<BridgeState*>(particle));
}

int Bridge::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

Belief* Bridge::Tau(const Belief* belief, int action, OBS_TYPE obs) const {
	static vector<double> probs = vector<double>(NumStates());

	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		BridgeState* state = static_cast<BridgeState*>(particles[i]);
		int next_pos = state->position;
		if (action == LEFT) {
			next_pos--;
			if (next_pos < 0)
				next_pos = 0;
		} else if (action == RIGHT) {
			if (next_pos < BRIDGELENGTH - 1)
				next_pos++;
			else
				continue;
		}

		double p = state->weight * (obs == 1);
		probs[next_pos] += p;
		sum += p;
	}

	vector<State*> new_particles;
	for (int i = 0; i < NumStates(); i++) {
		if (probs[i] > 0) {
			BridgeState* new_particle = static_cast<BridgeState*>(Allocate(i));
			new_particle->position = i;
			new_particle->weight = probs[i] / sum;
			new_particles.push_back(new_particle);
			probs[i] = 0;
		}
	}

	return new ParticleBelief(new_particles, this, NULL, false);
}

void Bridge::Observe(const Belief* belief, int action,
	map<OBS_TYPE, double>& obss) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();
	if (action == HELP)
		return;

	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		BridgeState* state = static_cast<BridgeState*>(particle);
		if (!(state->position == BRIDGELENGTH - 1 && action == RIGHT)) {
			obss[1] = 1.0;
			break;
		}
	}
}

double Bridge::StepReward(const Belief* belief, int action) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		BridgeState* state = static_cast<BridgeState*>(particle);
		double reward = -1;

		if (action == RIGHT && state->position == BRIDGELENGTH - 1) {
			reward = 0;
		} else if (action == HELP) {
			reward = -20 - state->position;
		}

		sum += state->weight * reward;
	}

	return sum;
}

class BridgePOMCPPrior: public POMCPPrior {
public:
	BridgePOMCPPrior(const DSPOMDP* model) :
		POMCPPrior(model) {
	}

	void ComputePreference(const State& state) {
		for (int a = 0; a < 3; a++) {
			legal_actions_.push_back(a);
		}

		preferred_actions_.push_back(2);
	}
};

POMCPPrior* Bridge::CreatePOMCPPrior(string name) const {
	if (name == "UNIFORM") {
		return new UniformPOMCPPrior(this);
	} else if (name == "DEFAULT" || name == "HELP") {
		return new BridgePOMCPPrior(this);
	} else {
		cerr << "Unsupported POMCP prior: " << name << endl;
		exit(1);
		return NULL;
	}
}

} // namespace despot
