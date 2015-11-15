#ifndef ROCKSAMPLE_H
#define ROCKSAMPLE_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include "base/base_rock_sample.h"
#include <despot/util/coord.h>
#include <despot/util/grid.h>

namespace despot {

/* =============================================================================
 * RockSample class
 * =============================================================================*/

class RockSample: public BaseRockSample {
public:
	RockSample(std::string map);
	RockSample(int size, int rocks);

	bool Step(State& state, double rand_num, int action, double& reward,
		OBS_TYPE& obs) const;
	int NumActions() const;
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;
	void PrintObs(const State& state, OBS_TYPE observation,
		std::ostream& out = std::cout) const;
};

} // namespace despot

#endif
