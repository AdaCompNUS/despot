#ifndef SEEDS_H
#define SEEDS_H

#include <vector>
#include "random.h"

using namespace std;

class Seeds {
private:
	static unsigned root_seed_;
	static Random seed_gen_;
	static int num_assigned_seeds_;

public:
	static void root_seed(unsigned value);

	static unsigned Next();

	static vector<unsigned> Next(int n);
};

#endif
