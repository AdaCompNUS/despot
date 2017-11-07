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
/**
 * [Optional]
 * Design your custom belief by inheriting this class
 */
class Belief {
public:
	const DSPOMDP* model_;
	History history_;

public:
	Belief(const DSPOMDP* model);
	virtual ~Belief();

	/**
	 * Sample states from a belief.
	 * Returns a set of sampled states.
	 *
	 * @param num Number of states to be sampled
	 */
	virtual std::vector<State*> Sample(int num) const = 0;

	/**
	 * Update the belief.
	 *
	 * @param action The action taken in the last step
	 * @param obs    The observation received in the last step
	 */
	virtual void Update(ACT_TYPE action, OBS_TYPE obs) = 0;

	virtual std::string text() const;
	friend std::ostream& operator<<(std::ostream& os, const Belief& belief);

	/**
	 * Returns a copy of this belief.
	 */
	virtual Belief* MakeCopy() const = 0;

};

} // namespace despot

#endif
