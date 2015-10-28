#ifndef ROCKSAMPLE_H
#define ROCKSAMPLE_H

#include "core/pomdp.h"
#include "core/mdp.h"
#include "base/base_rock_sample.h"
#include "util/coord.h"
#include "util/grid.h"

/* =============================================================================
 * RockSample class
 * =============================================================================*/

class RockSample: public BaseRockSample {
public:
	RockSample(string map);
	RockSample(int size, int rocks);

	bool Step(State& state, double rand_num, int action, double& reward,
		OBS_TYPE& obs) const;
	int NumActions() const;
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;
	void PrintObs(const State& state, OBS_TYPE observation,
		ostream& out = cout) const;
};

#endif
