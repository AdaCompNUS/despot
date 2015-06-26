#ifndef DIRICHLET_H
#define DIRICHLET_H

#include <vector>
#include "gamma.h"

using namespace std;

class Dirichlet {
private:
	vector<double> alpha_;
public:
	Dirichlet(vector<double> alpha);

	vector<double> alpha();
	vector<double> Next();
	static vector<double> Next(vector<double> alpha);
};

#endif

