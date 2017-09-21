#ifndef BUILTIN_UPPER_BOUND_H
#define BUILTIN_UPPER_BOUND_H

#include <despot/interface/upper_bound.h>

namespace despot {

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
