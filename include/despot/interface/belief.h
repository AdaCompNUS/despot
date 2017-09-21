#ifndef BELIEF_H
#define BELIEF_H

#include <vector>

#include <despot/util/random.h>
#include <despot/util/logging.h>
#include <despot/core/history.h>

namespace despot {

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

	virtual std::vector<State*> Sample(int num) const = 0;
	virtual void Update(int action, OBS_TYPE obs) = 0;

	virtual std::string text() const;
	friend std::ostream& operator<<(std::ostream& os, const Belief& belief);
	virtual Belief* MakeCopy() const = 0;

};

} // namespace despot

#endif
