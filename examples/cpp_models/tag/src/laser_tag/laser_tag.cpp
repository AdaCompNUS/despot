#include "laser_tag.h"
#include <math.h>
#include <cmath>

using namespace std;

namespace despot {

const OBS_TYPE ONE = 1;
int LaserTag::NBEAMS = 8;
int LaserTag::BITS_PER_READING = 7;

/* =============================================================================
 * LaserTag class
 * =============================================================================*/

LaserTag::LaserTag() :
	BaseTag(),
	noise_sigma_(2.5),
	unit_size_(1.0) {
	istringstream iss(RandomMap(7, 11, 8));
	BaseTag::Init(iss);
	Init();
  robot_pos_unknown_ = false;
}

LaserTag::LaserTag(string params_file) :
	BaseTag(params_file),
	noise_sigma_(2.5),
	unit_size_(1.0) {
	Init();
  robot_pos_unknown_ = false;
}

double LaserTag::LaserRange(const State& state, int dir) const {
	Coord rob = floor_.GetCell(rob_[state.state_id]), opp = floor_.GetCell(
		opp_[state.state_id]);
	int d = 1;
	while (true) {
		Coord coord = rob + Compass::DIRECTIONS[dir] * d;
		if (floor_.GetIndex(coord) == -1 || coord == opp)
			break;
		d++;
	}
	int x = Compass::DIRECTIONS[dir].x, y = Compass::DIRECTIONS[dir].y;

	return d * sqrt(x * x + y * y);
}

void LaserTag::Init() {
	for (int i = 0; i < NBEAMS; i++)
		SetReading(same_loc_obs_, 101, i);

	reading_distributions_.resize(NumStates());

	for (int s = 0; s < NumStates(); s++) {
		reading_distributions_[s].resize(NBEAMS);

		for (int d = 0; d < NBEAMS; d++) {
			double dist = LaserRange(*states_[s], d);
			for (int reading = 0; reading < dist / unit_size_; reading++) {
				double min_noise = reading * unit_size_ - dist;
				double max_noise = min(dist, (reading + 1) * unit_size_) - dist;
				double prob =
					2
						* (gausscdf(max_noise, 0, noise_sigma_)
							- (reading > 0 ?
								gausscdf(min_noise, 0, noise_sigma_) : 0 /*min_noise = -infty*/));

				reading_distributions_[s][d].push_back(prob);
			}
		}
	}
}

bool LaserTag::Step(State& state, double random_num, int action,
	double& reward) const {
	Random random(random_num);
	bool terminal = BaseTag::Step(state, random.NextDouble(), action, reward);
	return terminal;
}

bool LaserTag::Step(State& state, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	Random random(random_num);
	bool terminal = BaseTag::Step(state, random.NextDouble(), action, reward);

	if (terminal) {
		obs = same_loc_obs_;
	} else {
		if (rob_[state.state_id] == opp_[state.state_id])
			obs = same_loc_obs_;
		else {
			const vector<vector<double> >& distribution = reading_distributions_[state.state_id];

			obs = 0;
			for (int dir = 0; dir < NBEAMS; dir++) {
				double mass = random.NextDouble();
				int reading = 0;
				for (; reading < distribution[dir].size(); reading++) {
					mass -= distribution[dir][reading];
					if (mass < Globals::TINY)
						break;
				}
				SetReading(obs, reading, dir);
			}
		}
	}

	return terminal;
}

double LaserTag::ObsProb(OBS_TYPE obs, const State& state, int action) const {
	if (rob_[state.state_id] == opp_[state.state_id])
		return obs == same_loc_obs_;

	double prod = 1.0;
	for (int dir = 0; dir < NBEAMS; dir++) {
		int reading = GetReading(obs, dir);
		if (reading >= LaserRange(state, dir) / unit_size_)
			return 0;
		double prob_mass = reading_distributions_[state.state_id][dir][reading];
		prod *= prob_mass;
	}

	return prod;
}

void LaserTag::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	for (int i = 0; i < NBEAMS; i++)
		out << GetReading(obs, i) << " ";
	out << endl;
}

int LaserTag::GetReading(OBS_TYPE obs, OBS_TYPE dir) {
	return (obs >> (dir * BITS_PER_READING)) & ((ONE << BITS_PER_READING) - 1);
}

void LaserTag::SetReading(OBS_TYPE& obs, OBS_TYPE reading, OBS_TYPE dir) {
	// Clear bits
	obs &= ~(((ONE << BITS_PER_READING) - 1) << (dir * BITS_PER_READING));
	// Set bits
	obs |= reading << (dir * BITS_PER_READING);
}

int LaserTag::GetBucket(double noisy) const {
	return (int) std::floor(noisy / unit_size_);
}

Belief* LaserTag::InitialBelief(const State* start, string type) const {
	assert(start != NULL);

	vector<State*> particles;
	int N = floor_.NumCells();
	double wgt = 1.0 / N / N;
	for (int rob = 0; rob < N; rob++) {
		for (int opp = 0; opp < N; opp++) {
			TagState* state = static_cast<TagState*>(Allocate(
				RobOppIndicesToStateIndex(rob, opp), wgt));
			particles.push_back(state);
		}
	}

	ParticleBelief* belief = new ParticleBelief(particles, this);
	belief->state_indexer(this);
	return belief;
}

ostream& operator<<(ostream& os, const LaserTag& lasertag) {
	for (int s = 0; s < lasertag.NumStates(); s++) {
		os << "State " << s << " " << lasertag.opp_[s] << " "
			<< lasertag.rob_[s] << " " << lasertag.floor_.NumCells() << endl;
		lasertag.PrintState(*lasertag.states_[s], os);

		for (int d = 0; d < lasertag.NBEAMS; d++) {
			os << d;
			for (int i = 0; i < lasertag.reading_distributions_[s][d].size(); i++)
				os << " " << lasertag.reading_distributions_[s][d][i];
			os << endl;
		}
	}
	return os;
}

void LaserTag::Observe(const Belief* belief, int action,
	map<OBS_TYPE, double>& obss) const {
	cerr << "Exit: Two many observations!" << endl;
	exit(0);
}

} // namespace despot
