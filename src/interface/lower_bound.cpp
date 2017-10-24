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

ScenarioLowerBound::ScenarioLowerBound(const DSPOMDP* model) :
	model_(model){
}

void ScenarioLowerBound::Init(const RandomStreams& streams) {
}

void ScenarioLowerBound::Reset() {
}

void ScenarioLowerBound::Learn(VNode* tree) {
}

/* =============================================================================
 * ParticleLowerBound class
 * =============================================================================*/

ParticleLowerBound::ParticleLowerBound(const DSPOMDP* model) :
	ScenarioLowerBound(model) {
}

ValuedAction ParticleLowerBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	return Value(particles);
}

/* =============================================================================
 * BeliefLowerBound class
 * =============================================================================*/

BeliefLowerBound::BeliefLowerBound(const DSPOMDP* model) :
	model_(model) {
}

void BeliefLowerBound::Learn(VNode* tree) {
}

} // namespace despot
