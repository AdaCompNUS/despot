#ifndef POLICY_H
#define POLICY_H

#include <vector>

#include "random_streams.h"
#include "lower_bound.h"
#include "util/random.h"
#include "history.h"

#include "string.h"
#include <queue>
#include <vector>
#include <stdlib.h>
#include "globals.h"
#include "pomdp.h"

using namespace std;

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

	friend ostream& operator<<(ostream& os, const ValuedAction& va);
};

/* =============================================================================
 * Policy class
 * =============================================================================*/

class Policy: public ScenarioLowerBound {
private:
	mutable int initial_depth_;
	ParticleLowerBound* particle_lower_bound_;

	ValuedAction RecursiveValue(const vector<State*>& particles,
		RandomStreams& streams, History& history) const;

public:
	Policy(const DSPOMDP* model, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);
	virtual ~Policy();

	void Reset();
	virtual int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const = 0;

	ParticleLowerBound* particle_lower_bound() const;

	ValuedAction Value(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	virtual ValuedAction Search();
};

/* =============================================================================
 * BlindPolicy class
 * =============================================================================*/

class BlindPolicy: public Policy {
private:
	int action_;

public:
	BlindPolicy(const DSPOMDP* model, int action, ParticleLowerBound*
		particle_lower_bound, Belief* belief = NULL);

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(int action, OBS_TYPE obs);
};

/* =============================================================================
 * RandomPolicy class
 * =============================================================================*/

class RandomPolicy: public Policy {
private:
	vector<double> action_probs_;

public:
	RandomPolicy(const DSPOMDP* model, ParticleLowerBound* ParticleLowerBound,
		Belief* belief = NULL);
	RandomPolicy(const DSPOMDP* model, const vector<double>& action_probs,
		ParticleLowerBound* ParticleLowerBound,
		Belief* belief = NULL);

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(int action, OBS_TYPE obs);
};

/* =============================================================================
 * ModeStatePolicy class
 * =============================================================================*/

class ModeStatePolicy: public Policy {
private:
	const StateIndexer& indexer_;
	const StatePolicy& policy_;
	mutable vector<double> state_probs_;

public:
	ModeStatePolicy(const DSPOMDP* model, const StateIndexer& indexer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MMAPStatePolicy class
 * =============================================================================*/

class MMAPStatePolicy: public Policy { // Marginal MAP state policy
private:
	const MMAPInferencer& inferencer_;
	const StatePolicy& policy_;

public:
	MMAPStatePolicy(const DSPOMDP* model, const MMAPInferencer& inferencer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MajorityActionPolicy class
 * =============================================================================*/

class MajorityActionPolicy: public Policy {
private:
	const StatePolicy& policy_;

public:
	MajorityActionPolicy(const DSPOMDP* model, const StatePolicy& policy,
		ParticleLowerBound* particle_lower_bound, Belief* belief = NULL);

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

#endif
