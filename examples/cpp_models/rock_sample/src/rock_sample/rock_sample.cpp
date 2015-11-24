#include "rock_sample.h"

using namespace std;

namespace despot {

/* =============================================================================
 * RockSample class
 * =============================================================================*/

RockSample::RockSample(string map) :
	BaseRockSample(map) {
	half_efficiency_distance_ = 20;
}

RockSample::RockSample(int size, int rocks) :
	BaseRockSample(size, rocks) {
	half_efficiency_distance_ = 20;
}

bool RockSample::Step(State& state, double rand_num, int action, double& reward,
	OBS_TYPE& obs) const {
	RockSampleState& rockstate = static_cast<RockSampleState&>(state);
	reward = 0;
	obs = E_NONE;

	if (action < E_SAMPLE) { // Move
		switch (action) {
		case Compass::EAST:
			if (GetX(&rockstate) + 1 < size_) {
				IncX(&rockstate);
				break;
			} else {
				reward = +10;
				return true;
			}

		case Compass::NORTH:
			if (GetY(&rockstate) + 1 < size_)
				IncY(&rockstate);
			else
				reward = -100;
			break;

		case Compass::SOUTH:
			if (GetY(&rockstate) - 1 >= 0)
				DecY(&rockstate);
			else
				reward = -100;
			break;

		case Compass::WEST:
			if (GetX(&rockstate) - 1 >= 0)
				DecX(&rockstate);
			else
				reward = -100;
			break;
		}
	}

	if (action == E_SAMPLE) { // Sample
		int rock = grid_(GetRobPosIndex(&rockstate));
		if (rock >= 0) {
			if (GetRock(&rockstate, rock))
				reward = +10;
			else
				reward = -10;
			SampleRock(&rockstate, rock);
		} else {
			reward = -100;
		}
	}

	if (action > E_SAMPLE) { // Sense
		int rock = action - E_SAMPLE - 1;
		assert(rock < num_rocks_);
		obs = GetObservation(rand_num, rockstate, rock);
	}

	// assert(reward != -100);
	return false;
}

int RockSample::NumActions() const {
	return num_rocks_ + 5;
}

double RockSample::ObsProb(OBS_TYPE obs, const State& state, int action) const {
	if (action <= E_SAMPLE)
		return obs == E_NONE;

	if (obs != E_GOOD && obs != E_BAD)
		return 0;

	const RockSampleState& rockstate =
		static_cast<const RockSampleState&>(state);

	int rock = action - E_SAMPLE - 1;
	double distance = Coord::EuclideanDistance(GetRobPos(&rockstate),
		rock_pos_[rock]);
	double efficiency = (1 + pow(2, -distance / half_efficiency_distance_))
		* 0.5;

	return
		((GetRock(&rockstate, rock) & 1) == obs) ? efficiency : (1 - efficiency);
}

void RockSample::PrintObs(const State& state, OBS_TYPE observation,
	ostream& out) const {
	switch (observation) {
	case E_NONE:
		out << "None" << endl;
		break;
	case E_GOOD:
		out << "Good" << endl;
		break;
	case E_BAD:
		out << "Bad" << endl;
		break;
	}
}

} // namespace despot
