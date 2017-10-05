#ifndef POLICY_H
#define POLICY_H

#include <vector>

#include <despot/random_streams.h>
#include <despot/interface/lower_bound.h>
#include <despot/util/random.h>
#include <despot/core/history.h>

#include <string.h>
#include <queue>
#include <vector>
#include <stdlib.h>
#include <despot/core/globals.h>
#include <despot/interface/pomdp.h>

namespace despot {

class State;
class StateIndexer;
class StatePolicy;
class DSPOMDP;
class MMAPInferencer;

/* =============================================================================
 * ValuedAction struct
 * =============================================================================*/

struct ValuedAction {
	int action;
	double value;

	ValuedAction();
	ValuedAction(int _action, double _value);

	friend std::ostream& operator<<(std::ostream& os, const ValuedAction& va);
};

/* =============================================================================
 * Policy class
 * =============================================================================*/
/**
 * [Optional interface] Design your custom default policy by inheriting this class
 */
class Policy: public ScenarioLowerBound {
private:
	mutable int initial_depth_;
	ParticleLowerBound* particle_lower_bound_;

	ValuedAction RecursiveValue(const std::vector<State*>& particles,
		RandomStreams& streams, History& history) const;

public:
	Policy(const DSPOMDP* model, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);
	virtual ~Policy();

	void Reset();
	virtual int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const = 0;

	ParticleLowerBound* particle_lower_bound() const;

	ValuedAction Value(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	virtual ValuedAction Search();
};

} // namespace despot

#endif
