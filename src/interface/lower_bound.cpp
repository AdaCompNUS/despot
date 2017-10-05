#include <despot/interface/lower_bound.h>
#include <despot/interface/pomdp.h>
#include <despot/core/node.h>
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * ValuedAction class
 * =============================================================================*/

ValuedAction::ValuedAction() :
	action(-1),
	value(0) {
}

ValuedAction::ValuedAction(ACT_TYPE _action, double _value) :
	action(_action),
	value(_value) {
}

ostream& operator<<(ostream& os, const ValuedAction& va) {
	os << "(" << va.action << ", " << va.value << ")";
	return os;
}

/* =============================================================================
 * ScenarioLowerBound class
 * =============================================================================*/

ScenarioLowerBound::ScenarioLowerBound(const DSPOMDP* model, Belief* belief) :
	Solver(model, belief) {
}

void ScenarioLowerBound::Init(const RandomStreams& streams) {
}

void ScenarioLowerBound::Reset() {
}

ValuedAction ScenarioLowerBound::Search() {
	RandomStreams streams(Globals::config.num_scenarios,
		Globals::config.search_depth);
	vector<State*> particles = belief_->Sample(Globals::config.num_scenarios);

	ValuedAction va = Value(particles, streams, history_);

	for (int i = 0; i < particles.size(); i++)
		model_->Free(particles[i]);

	return va;
}

void ScenarioLowerBound::Learn(VNode* tree) {
}

/* =============================================================================
 * ParticleLowerBound class
 * =============================================================================*/

ParticleLowerBound::ParticleLowerBound(const DSPOMDP* model, Belief* belief) :
	ScenarioLowerBound(model, belief) {
}

ValuedAction ParticleLowerBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	return Value(particles);
}

/* =============================================================================
 * BeliefLowerBound class
 * =============================================================================*/

BeliefLowerBound::BeliefLowerBound(const DSPOMDP* model, Belief* belief) :
	Solver(model, belief) {
}

ValuedAction BeliefLowerBound::Search() {
	return Value(belief_);
}

void BeliefLowerBound::Learn(VNode* tree) {
}

} // namespace despot
