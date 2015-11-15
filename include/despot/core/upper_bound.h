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
 * TrivialParticleUpperBound class
 * =============================================================================*/

class TrivialParticleUpperBound: public ParticleUpperBound {
protected:
	const DSPOMDP* model_;
public:
	TrivialParticleUpperBound(const DSPOMDP* model);
	virtual ~TrivialParticleUpperBound();

	double Value(const State& state) const;

	virtual double Value(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;
};

/* =============================================================================
 * LookaheadUpperBound class
 * =============================================================================*/

class LookaheadUpperBound: public ScenarioUpperBound {
protected:
	const DSPOMDP* model_;
	const StateIndexer& indexer_;
	std::vector<std::vector<std::vector<double> > > bounds_;
	ParticleUpperBound* particle_upper_bound_;

public:
	LookaheadUpperBound(const DSPOMDP* model, const StateIndexer& indexer,
		ParticleUpperBound* bound);

	virtual void Init(const RandomStreams& streams);

	double Value(const std::vector<State*>& particles,
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

/* =============================================================================
 * TrivialBeliefUpperBound class
 * =============================================================================*/

class TrivialBeliefUpperBound: public BeliefUpperBound {
protected:
	const DSPOMDP* model_;
public:
	TrivialBeliefUpperBound(const DSPOMDP* model);

	double Value(const Belief* belief) const;
};

/* =============================================================================
 * MDPUpperBound class
 * =============================================================================*/

class MDPUpperBound: public ParticleUpperBound, public BeliefUpperBound {
protected:
	const MDP* model_;
	const StateIndexer& indexer_;
	std::vector<ValuedAction> policy_;

public:
	MDPUpperBound(const MDP* model, const StateIndexer& indexer);

  // shut off "hides overloaded virtual function" warning
  using ParticleUpperBound::Value;
	double Value(const State& state) const;

	double Value(const Belief* belief) const;
};

} // namespace despot

#endif
