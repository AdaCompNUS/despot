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

class BeliefUpperBound {
public:
	BeliefUpperBound();
	virtual ~BeliefUpperBound();

	virtual double Value(const Belief* belief) const = 0;
};

} // namespace despot

#endif
