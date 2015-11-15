#ifndef LOWER_BOUND_H
#define LOWER_BOUND_H

#include <vector>
#include <despot/random_streams.h>
#include <despot/core/history.h>
#include <despot/core/solver.h>

namespace despot {

class State;
class DSPOMDP;
class VNode;

/* =============================================================================
 * ScenarioLowerBound class
 * =============================================================================*/

/**
 * Interface for an algorithm computing a lower bound for the maximum total
 * discounted reward over obtainable by a policy on a set of weighted scenarios.
 * The horizon is infinite. The first action that need to be followed to obtain
 * the bound is also returned.
 */
class ScenarioLowerBound: public Solver {
public:
	ScenarioLowerBound(const DSPOMDP* model, Belief* belief = NULL);

	virtual void Init(const RandomStreams& streams);

	virtual ValuedAction Search();
	virtual void Learn(VNode* tree);
	virtual void Reset();

	/**
	 * Returns a lower bound for the maximum total discounted reward obtainable
	 * by a policy on a set of weighted scenarios. The horizon is infinite. The
	 * first action that need to be followed to obtain the bound is also
	 * returned.
	 *
	 * @param particles Particles in the scenarios.
	 * @param streams Random numbers attached to the scenarios.
	 * @param history Current action-observation history.
	 * @return (a, v), where v is the lower bound and a is the first action needed
	 * to obtain the lower bound.
	 */
	virtual ValuedAction Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const = 0;
};

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
 * ParticleLowerBound class
 * =============================================================================*/

/**
 * Interface for an algorithm computing a lower bound for maximum total
 * discounted reward obtainable by a policy on a set of weighted scenarios with
 * only the particles given. The horizon is inifnite. The first action that need
 * to be followed to obtain the bound is also returned.
 */
class ParticleLowerBound : public ScenarioLowerBound {
public:
	ParticleLowerBound(const DSPOMDP* model, Belief* belief = NULL);

	/**
	 * Returns a lower bound for the maximum total discounted reward obtainable
	 * by a policy on a set of particles. The horizon is infinite. The horizon is
	 * inifnite. The first action that need to be followed to obtain the bound is
	 * also returned.
	 */
	virtual ValuedAction Value(const std::vector<State*>& particles) const = 0;

	ValuedAction Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;
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
 * BeliefLowerBound class
 * =============================================================================*/

/**
 * Interface for an algorithm used to compute a lower bound for the infinite
 * horizon reward that can be obtained by the optimal policy on a belief.
 */
class BeliefLowerBound: public Solver {
public:
	BeliefLowerBound(const DSPOMDP* model, Belief* belief = NULL);

	virtual ValuedAction Search();
	virtual void Learn(VNode* tree);

	virtual ValuedAction Value(const Belief* belief) const = 0;
};

/* =============================================================================
 * TrivialBeliefLowerBound class
 * =============================================================================*/

class TrivialBeliefLowerBound: public BeliefLowerBound {
public:
	TrivialBeliefLowerBound(const DSPOMDP* model, Belief* belief = NULL);

	virtual ValuedAction Value(const Belief* belief) const;
};

} // namespace despot

#endif
