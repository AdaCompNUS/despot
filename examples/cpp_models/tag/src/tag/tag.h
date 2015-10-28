#ifndef TAG_H
#define TAG_H

#include "core/pomdp.h"
#include "core/mdp.h"
#include "util/coord.h"
#include "util/floor.h"
#include "base/base_tag.h"

/* =============================================================================
 * Tag class
 * =============================================================================*/

class Tag: public BaseTag {
private:
	vector<OBS_TYPE> obs_;
public:
	Tag();
	Tag(string params_file);

	bool Step(State& state, double random_num, int action, double& reward,
		OBS_TYPE& obs) const;

	double ObsProb(OBS_TYPE obs, const State& state, int action) const;

	Belief* ExactPrior() const;
	Belief* ApproxPrior() const;
	Belief* InitialBelief(const State* start, string type = "DEFAULT") const;

	void Observe(const Belief* belief, int action,
		map<OBS_TYPE, double>& obss) const;

	void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const;
};

#endif
