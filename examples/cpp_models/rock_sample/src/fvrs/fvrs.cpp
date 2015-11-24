#include "fvrs.h"

using namespace std;

namespace despot {

/* =============================================================================
 * FVRS class
 * =============================================================================*/

FVRS::FVRS(string map) :
	BaseRockSample(map) {
	half_efficiency_distance_ = (size_ - 1) * sqrt(2) / 4 / 2;
}

FVRS::FVRS(int size, int rocks) :
	BaseRockSample(size, rocks) {
	half_efficiency_distance_ = (size_ - 1) * sqrt(2) / 4 / 2;
}

bool FVRS::Step(State& state, double rand_num, int action, double& reward,
	OBS_TYPE& obs) const {
	RockSampleState& rockstate = static_cast<RockSampleState&>(state);
	reward = 0;

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

	obs = GetObservation(rand_num, rockstate);

	// assert(reward != -100);
	return false;
}

int FVRS::NumActions() const {
	return 5;
}

double FVRS::ObsProb(OBS_TYPE obs, const State& state, int action) const {
	const RockSampleState& rockstate =
		static_cast<const RockSampleState&>(state);

	double prob = 1.0;
	for (int rock = 0; rock < num_rocks_; rock++) {
		double distance = Coord::EuclideanDistance(GetRobPos(&rockstate),
			rock_pos_[rock]);
		double efficiency = (1 + pow(2, -distance / half_efficiency_distance_))
			* 0.5;
		bool bit = CheckFlag(obs, rock);
		prob *= (
			((GetRock(&rockstate, rock) & 1) == bit) ?
				efficiency : (1 - efficiency));
	}
	return prob;
}

int FVRS::GetObservation(double rand_num,
	const RockSampleState& rockstate) const {
	int obs = 0;
	for (int rock = 0; rock < num_rocks_; rock++) {
		double distance = Coord::EuclideanDistance(GetRobPos(&rockstate),
			rock_pos_[rock]);
		double efficiency = (1 + pow(2, -distance / half_efficiency_distance_))
			* 0.5;

		// cout << efficiency << endl;
		bool bit = 0;
		if (rand_num < efficiency) {
			bit = GetRock(&rockstate, rock) & E_GOOD;
		} else {
			bit = !(GetRock(&rockstate, rock) & E_GOOD);

			rand_num -= efficiency;
			if (efficiency < 1.0)
				rand_num /= (1.0 - rand_num);
		}

		if (bit)
			SetFlag(obs, rock);
	}
	return obs;
}

void FVRS::PrintObs(const State& state, OBS_TYPE observation,
	ostream& out) const {
	for (int rock = 0; rock < num_rocks_; rock++)
		out << CheckFlag(observation, rock);
	out << endl;
}

} // namespace despot
