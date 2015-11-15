#include <despot/util/seeds.h>

using namespace std;

namespace despot {

int Seeds::num_assigned_seeds_ = 0;
unsigned Seeds::root_seed_ = 0;
Random Seeds::seed_gen_ = Random((unsigned) 0);

void Seeds::root_seed(unsigned value) {
	root_seed_ = value;
	seed_gen_ = Random(root_seed_);
}

unsigned Seeds::Next() {
	// return root_seed_ ^ (num_assigned_seeds_++);
	return seed_gen_.NextUnsigned();
}

vector<unsigned> Seeds::Next(int n) {
	vector<unsigned> seeds;
	for (int i = 0; i < n; i++)
		seeds.push_back(Next());
	return seeds;
}

} // namespace despot
