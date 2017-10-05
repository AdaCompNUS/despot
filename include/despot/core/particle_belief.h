#ifndef PARTICLE_BELIEF_H
#define PARTICLE_BELIEF_H

#include <despot/interface/belief.h>
#include <vector>

#include <despot/util/random.h>
#include <despot/util/logging.h>
#include <despot/core/history.h>

namespace despot {

class State;
class StateIndexer;
class DSPOMDP;

/* =============================================================================
 * ParticleBelief class
 * =============================================================================*/

class ParticleBelief: public Belief {
protected:
	std::vector<State*> particles_;
	int num_particles_;
	Belief* prior_;
	bool split_;
	std::vector<State*> initial_particles_;
	const StateIndexer* state_indexer_;

public:
	ParticleBelief(std::vector<State*> particles, const DSPOMDP* model,
		Belief* prior = NULL, bool split = true);

	virtual ~ParticleBelief();
	void state_indexer(const StateIndexer* indexer);

	virtual const std::vector<State*>& particles() const;
	virtual std::vector<State*> Sample(int num) const;

	virtual void Update(ACT_TYPE action, OBS_TYPE obs);

	virtual Belief* MakeCopy() const;

	virtual std::string text() const;

	static std::vector<State*> Sample(int num, std::vector<State*> belief,
		const DSPOMDP* model);
	static std::vector<State*> Resample(int num, const std::vector<State*>& belief,
		const DSPOMDP* model, History history, int hstart = 0);
	static std::vector<State*> Resample(int num, const Belief& belief,
		History history, int hstart = 0);
	static std::vector<State*> Resample(int num, const DSPOMDP* model,
		const StateIndexer* indexer, ACT_TYPE action, OBS_TYPE obs);
};

} // namespace despot

#endif
