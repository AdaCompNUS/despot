#ifndef SEEDS_H
#define SEEDS_H

#include <vector>
#include <despot/util/random.h>

namespace despot {

class Seeds {
private:
	static unsigned root_seed_;
	static Random seed_gen_;
	static int num_assigned_seeds_;

public:
	static void root_seed(unsigned value);

	static unsigned Next();

	static std::vector<unsigned> Next(int n);
};

} // namespace despot

#endif
