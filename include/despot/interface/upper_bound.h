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
 * [Optional interface] Interface for an algorithm computing a upper bound for the optimal total
 * discounted reward on a set of weighted scenarios.
 * The horizon is infinite.
 */
class ScenarioUpperBound {
public:
	ScenarioUpperBound();
	virtual ~ScenarioUpperBound();

	virtual void Init(const RandomStreams& streams);

	virtual double Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const = 0;
};

/* =============================================================================
 * ParticleUpperBound class
 * =============================================================================*/
/**
 * [Optional interface] Interface for an algorithm computing a upper bound for optimal total
 * discounted reward on a set of weighted scenarios with
 * only the particles given. The horizon is inifnite.
 */
class ParticleUpperBound : public ScenarioUpperBound {
public:
	ParticleUpperBound();
	virtual ~ParticleUpperBound();

	/**
	 * Returns an upper bound to the maximum total discounted reward over an
	 * infinite horizon for the (unweighted) particle.
	 */
	virtual double Value(const State& state) const = 0;

	virtual double Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;
};


/* =============================================================================
 * BeliefUpperBound class
 * =============================================================================*/
/**
 * [Optional interface] Interface for an algorithm used to compute a upper bound for the infinite
 * horizon reward that can be obtained by the optimal policy on a belief.
 */
class BeliefUpperBound {
public:
	BeliefUpperBound();
	virtual ~BeliefUpperBound();

	virtual double Value(const Belief* belief) const = 0;
};

} // namespace despot

#endif
