#ifndef FVRS_H
#define FVRS_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include <despot/util/coord.h>
#include <despot/util/grid.h>
#include "base/base_rock_sample.h"

/* =============================================================================
 * FVRS class
 * =============================================================================*/

class FVRS: public BaseRockSample {
public:
	FVRS(string map);
	FVRS(int size, int rocks);

	bool Step(State& state, double rand_num, int action, double& reward,
		OBS_TYPE& obs) const;
	int NumActions() const;
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;
	int GetObservation(double rand_num, const RockSampleState& rockstate) const;
	void PrintObs(const State& state, OBS_TYPE observation,
		ostream& out = cout) const;
};

#endif
