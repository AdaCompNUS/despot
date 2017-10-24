#ifndef BUILTIN_LOWER_BOUND_H
#define BUILTIN_LOWER_BOUND_H

#include <despot/interface/lower_bound.h>

namespace despot {

/* =============================================================================
 * POMCPScenarioLowerBound class
 * =============================================================================*/

class POMCPPrior;
class POMCPScenarioLowerBound: public ScenarioLowerBound {
private:
	double explore_constant_;
	POMCPPrior* prior_;

protected:
	double Simulate(State* particle, RandomStreams& streams, VNode* vnode,
		History& history) const;
	double Rollout(State* particle, RandomStreams& streams, int depth,
		History& history) const;
	VNode* CreateVNode(const History& history, int depth) const;

public:
	POMCPScenarioLowerBound(const DSPOMDP* model, POMCPPrior* prior,
		Belief* belief = NULL);

	ValuedAction Value(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * TrivialParticleLowerBound class
 * =============================================================================*/

class TrivialParticleLowerBound: public ParticleLowerBound {
public:
	TrivialParticleLowerBound(const DSPOMDP* model);

public:
	virtual ValuedAction Value(const std::vector<State*>& particles) const;
};

/* =============================================================================
 * TrivialBeliefLowerBound class
 * =============================================================================*/

class TrivialBeliefLowerBound: public BeliefLowerBound {
public:
	TrivialBeliefLowerBound(const DSPOMDP* model);

	virtual ValuedAction Value(const Belief* belief) const;
};

} // namespace despot

#endif
