#include <despot/core/builtin_lower_bounds.h>
#include <despot/core/builtin_policy.h>
#include <despot/core/builtin_upper_bounds.h>
#include <despot/interface/default_policy.h>
#include <despot/interface/pomdp.h>
#include <despot/interface/lower_bound.h>
#include <despot/interface/upper_bound.h>
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * State class
 * =============================================================================*/

ostream& operator<<(ostream& os, const State& state) {
	os << "(state_id = " << state.state_id << ", weight = " << state.weight
			<< ", text = " << (&state)->text() << ")";
	return os;
}

State::State() :
		state_id(-1) {
}

State::State(int _state_id, double _weight) :
		state_id(_state_id), weight(_weight) {
}

State::~State() {
}

string State::text() const {
	return "AbstractState";
}

double State::Weight(const vector<State*>& particles) {
	double weight = 0;
	for (int i = 0; i < particles.size(); i++)
		weight += particles[i]->weight;
	return weight;
}
/* =============================================================================
 * StateIndexer class
 * =============================================================================*/
StateIndexer::~StateIndexer() {
}

/* =============================================================================
 * StatePolicy class
 * =============================================================================*/
StatePolicy::~StatePolicy() {
}

/* =============================================================================
 * MMAPinferencer class
 * =============================================================================*/
MMAPInferencer::~MMAPInferencer() {
}

/* =============================================================================
 * DSPOMDP class
 * =============================================================================*/

DSPOMDP::DSPOMDP() {
}

DSPOMDP::~DSPOMDP() {
}

double DSPOMDP::Reward(const State& state, ACT_TYPE action) const {
	//cerr << __FUNCTION__<<" is called, but hasn't been implemented. To fix this error, implement "<<__FUNCTION__<< endl;
	//exit(-1);
	return GetMaxReward() + 1;	//return an invalid reward
}

bool DSPOMDP::Step(State& state, ACT_TYPE action, double& reward,
		OBS_TYPE& obs) const {
	return Step(state, Random::RANDOM.NextDouble(), action, reward, obs);
}

bool DSPOMDP::Step(State& state, double random_num, ACT_TYPE action,
		double& reward) const {
	OBS_TYPE obs;
	return Step(state, random_num, action, reward, obs);
}

State* DSPOMDP::CreateStartState(std::string type) const{
	cerr << "Unimplemented function: DSPOMDP::CreateStartState" << endl;
	exit(1);
}

ParticleUpperBound* DSPOMDP::CreateParticleUpperBound(string name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleUpperBound(this);
	} else {
		if (name != "print")
			cerr << "Unsupported base upper bound: " << name << endl;
		cerr << "Supported types: TRIVIAL (default)" << endl;
		exit(1);
	}
}

ScenarioUpperBound* DSPOMDP::CreateScenarioUpperBound(string name,
		string particle_bound_name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleUpperBound(this);
	} else {
		if (name != "print")
			cerr << "Unsupported upper bound: " << name << endl;
		cerr << "Supported types: TRIVIAL (default)" << endl;
		exit(1);
		return NULL;
	}
}

ParticleLowerBound* DSPOMDP::CreateParticleLowerBound(string name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleLowerBound(this);
	} else {
		if (name != "print")
			cerr << "Unsupported particle lower bound: " << name << endl;
		cerr << "Supported types: TRIVIAL (default)" << endl;
		exit(1);
		return NULL;
	}
}

ScenarioLowerBound* DSPOMDP::CreateScenarioLowerBound(string name,
		string particle_bound_name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleLowerBound(this);
	} else if (name == "RANDOM") {
		return new RandomPolicy(this,
				CreateParticleLowerBound(particle_bound_name));
	} else {
		if (name != "print")
			cerr << "Unsupported lower bound: " << name << endl;
		cerr << "Supported types: TRIVIAL (default)" << endl;
		exit(1);
		return NULL;
	}
}

POMCPPrior* DSPOMDP::CreatePOMCPPrior(string name) const {
	if (name == "UNIFORM" || name == "DEFAULT") {
		return new UniformPOMCPPrior(this);
	} else {
		cerr << "Unsupported POMCP prior: " << name << endl;
		exit(1);
		return NULL;
	}
}

vector<State*> DSPOMDP::Copy(const vector<State*>& particles) const {
	vector<State*> copy;
	for (int i = 0; i < particles.size(); i++)
		copy.push_back(Copy(particles[i]));
	return copy;
}

/* =============================================================================
 * BeliefMDP classs
 * =============================================================================*/

BeliefMDP::BeliefMDP() {
}

BeliefMDP::~BeliefMDP() {
}

BeliefLowerBound* BeliefMDP::CreateBeliefLowerBound(string name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialBeliefLowerBound(this);
	} else {
		cerr << "Unsupported belief lower bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

BeliefUpperBound* BeliefMDP::CreateBeliefUpperBound(string name) const {
	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialBeliefUpperBound(this);
	} else {
		cerr << "Unsupported belief upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

} // namespace despot
