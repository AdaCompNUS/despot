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
 * [Optional]
 * Interface for an algorithm computing a lower bound for the maximum total
 * discounted reward over obtainable by a policy on a set of weighted scenarios.
 * The horizon is infinite. The first action that need to be followed to obtain
 * the bound is also returned.
 */
class ScenarioLowerBound{
protected:
	const DSPOMDP* model_;
public:
	ScenarioLowerBound(const DSPOMDP* model/*, Belief* belief = NULL*/);
	virtual ~ScenarioLowerBound(){};

	const DSPOMDP* model(){return model_;};

	virtual void Init(const RandomStreams& streams);

	virtual void Learn(VNode* tree);
	virtual void Reset();

	/**
	 * Returns a lower bound for the maximum total discounted reward obtainable
	 * by a policy on a set of weighted scenarios. The horizon is infinite. The
	 * first action that need to be followed to obtain the bound is also
	 * returned.
	 *
	 * @param particles States in the head of scenarios.
	 * @param streams Random numbers attached to the scenarios.
	 * @param history Current action-observation history.
	 * @return (a, v), where v is the lower bound and a is the first action needed
	 * to obtain the lower bound.
	 */
	virtual ValuedAction Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const = 0;
};

/* =============================================================================
 * ParticleLowerBound class
 * =============================================================================*/

/**
 * [Optional]
 * Interface for an algorithm computing a lower bound for maximum total
 * discounted reward obtainable by a policy on a set of weighted scenarios with
 * only the particles given. The horizon is infinite. The first action that need
 * to be followed to obtain the bound is also returned.
 */
class ParticleLowerBound : public ScenarioLowerBound {
public:
	ParticleLowerBound(const DSPOMDP* model/*, Belief* belief = NULL*/);

	/**
	 * Returns a lower bound for the maximum total discounted reward obtainable
	 * by a policy on a set of particles. The horizon is infinite. The horizon is
	 * inifnite. The first action that need to be followed to obtain the bound is
	 * also returned.
	 * @param particles States in the head of scenarios.
	 */
	virtual ValuedAction Value(const std::vector<State*>& particles) const = 0;

	ValuedAction Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;
};

/* =============================================================================
 * BeliefLowerBound class
 * =============================================================================*/

/**
 * [Optional]
 * Interface for an algorithm used to compute a lower bound for the infinite
 * horizon reward that can be obtained by the optimal policy on a belief.
 */
class BeliefLowerBound {
protected:
	const DSPOMDP* model_;
public:
	BeliefLowerBound(const DSPOMDP* model);
	virtual ~BeliefLowerBound(){};

	const DSPOMDP* model(){return model_;};

	virtual void Learn(VNode* tree);

	/**
	 * Returns a lower bound for the maximum total discounted reward obtainable
	 * by a policy on a general belief representation. The horizon is infinite.
	 * The first action that need to be followed to obtain the bound is also returned.
	 * @param belief The current belief to be evaluated.
	 */
	virtual ValuedAction Value(const Belief* belief) const = 0;
};

} // namespace despot

#endif
