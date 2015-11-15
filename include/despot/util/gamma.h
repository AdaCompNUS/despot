#ifndef GAMMA_H
#define GAMMA_H

#include <cmath>
#include <cstdlib>
#include <despot/util/random.h>

class Gamma {
private:
	double k_;
	double theta_;
public:
	Gamma(double k, double theta);

	double Next();

	static double Next(double k, double theta);
};

#endif
