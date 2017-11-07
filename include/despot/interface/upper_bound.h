#ifndef UPPER_BOUND_H
#define UPPER_BOUND_H

#include <vector>
#include <cassert>

#include <despot/random_streams.h>
#include <despot/core/history.h>

namespace despot {

class State;
class StateIndexer;
class DSPOMDP;
class Belief;
class MDP;
struct ValuedAction;

/* =============================================================================
 * ScenarioUpperBound class
 * =============================================================================*/
/**
 * [Optional]
 * Interface for an algorithm computing a upper bound for the optimal total
 * discounted reward on a set of weighted scenarios.
 * The horizon is infinite.
 */
class ScenarioUpperBound {
public:
	ScenarioUpperBound();
	virtual ~ScenarioUpperBound();

	virtual void Init(const RandomStreams& streams);

	/**
	 * Returns a upper bound for the maximum total discounted reward
	 * on a set of weighted scenarios. The horizon is infinite.
	 *
	 * @param particles States in the head of scenarios.
	 * @param streams Random numbers attached to the scenarios.
	 * @param history Current action-observation history.
	 */
	virtual double Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const = 0;
};

/* =============================================================================
 * ParticleUpperBound class
 * =============================================================================*/
/**
 * [Optional]
 * Interface for an algorithm computing a upper bound for optimal total
 * discounted reward on a set of weighted scenarios with
 * only the particles given. The horizon is inifnite.
 */
class ParticleUpperBound : public ScenarioUpperBound {
public:
	ParticleUpperBound();
	virtual ~ParticleUpperBound();

	/**
	 * Returns a upper bound for the maximum total discounted reward
	 * starting from a non-determinized particle. The horizon is infinite.
	 *
	 * @param state The particle (a sampled state) to be evaluated
	 */
	virtual double Value(const State& state) const = 0;

	/**
	 * Evaluate a set of scenarios purely using the particles
	 * Returns a upper bound for the maximum total discounted reward
	 * on a set of non-determinized particles. The horizon is infinite.
	 *
	 * @param particles States in the head of scenarios.
	 * @param streams Random numbers attached to the scenarios.
	 * @param history Current action-observation history.
	 */
	virtual double Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;
};


/* =============================================================================
 * BeliefUpperBound class
 * =============================================================================*/
/**
 * [Optional]
 * Interface for an algorithm used to compute a upper bound for the infinite
 * horizon reward that can be obtained by the optimal policy on a
 * general representation of belief.
 */
class BeliefUpperBound {
public:
	BeliefUpperBound();
	virtual ~BeliefUpperBound();

	/**
	 * Returns a upper bound for the maximum total discounted reward
	 * on a general representation of a belief. The horizon is infinite.
	 *
	 * @param belief The belief to be evaluated
	 */
	virtual double Value(const Belief* belief) const = 0;
};

} // namespace despot

#endif
