#ifndef BELIEF_H
#define BELIEF_H

#include <vector>

#include "util/random.h"
#include "util/logging.h"
#include "history.h"

class State;
class StateIndexer;
class DSPOMDP;

/* =============================================================================
 * Belief class
 * =============================================================================*/

class Belief {
public:
	const DSPOMDP* model_;
	History history_;

public:
	Belief(const DSPOMDP* model);
	virtual ~Belief();

	virtual vector<State*> Sample(int num) const = 0;
	virtual void Update(int action, OBS_TYPE obs) = 0;

	virtual string text() const;
	friend ostream& operator<<(ostream& os, const Belief& belief);
	virtual Belief* MakeCopy() const = 0;

	static vector<State*> Sample(int num, vector<State*> belief,
		const DSPOMDP* model);
	static vector<State*> Resample(int num, const vector<State*>& belief,
		const DSPOMDP* model, History history, int hstart = 0);
	static vector<State*> Resample(int num, const Belief& belief,
		History history, int hstart = 0);
	static vector<State*> Resample(int num, const DSPOMDP* model,
		const StateIndexer* indexer, int action, OBS_TYPE obs);
};

/* =============================================================================
 * ParticleBelief class
 * =============================================================================*/

class ParticleBelief: public Belief {
protected:
	vector<State*> particles_;
	int num_particles_;
	Belief* prior_;
	bool split_;
	vector<State*> initial_particles_;
	const StateIndexer* state_indexer_;

public:
	ParticleBelief(vector<State*> particles, const DSPOMDP* model,
		Belief* prior = NULL, bool split = true);

	virtual ~ParticleBelief();
	void state_indexer(const StateIndexer* indexer);

	virtual const vector<State*>& particles() const;
	virtual vector<State*> Sample(int num) const;

	virtual void Update(int action, OBS_TYPE obs);

	virtual Belief* MakeCopy() const;

	virtual string text() const;
};

#endif
