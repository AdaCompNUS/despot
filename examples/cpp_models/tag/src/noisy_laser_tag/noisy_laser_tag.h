#ifndef NOISYLASERTAG_H
#define NOISYLASERTAG_H

#include "base/base_tag.h"

/* =============================================================================
 * NoisyLaserTag class
 * =============================================================================*/

class NoisyLaserTag: public BaseTag {
private:
	static int NBEAMS;
	static int BITS_PER_READING;

	double noise_sigma_;
	double unit_size_;
	vector<vector<vector<double> > > reading_distributions_;

public:
	NoisyLaserTag();
	NoisyLaserTag(string params_file);
	double LaserRange(const State& state, int dir) const;
	void Init();
	int GetReading(int);

	bool Step(State& state, double random_num, int action, double& reward) const;
	bool Step(State& state, double random_num, int action,
		double& reward, OBS_TYPE& obs) const;
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;

	Belief* InitialBelief(const State* start, string type = "DEFAULT") const;

	void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const;

	static int GetReading(OBS_TYPE obs, OBS_TYPE dir);
	static void SetReading(OBS_TYPE& obs, OBS_TYPE reading, OBS_TYPE dir);
	int GetBucket(double noisy) const;

	void Observe(const Belief* belief, int action, map<OBS_TYPE, double>& obss) const;

	friend ostream& operator<<(ostream& os, const NoisyLaserTag& lasertag);
};

#endif
