#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace despot {

struct Config {
	int search_depth;
	double discount;
	unsigned int root_seed;
	double time_per_move;  // CPU time available to construct the search tree
	int num_scenarios;
	double pruning_constant;
	double xi; // xi * gap(root) is the target uncertainty at the root.
	int sim_len; // Number of steps to run the simulation for.
  std::string default_action;
	int max_policy_sim_len; // Maximum number of steps for simulating the default policy
	double noise;
	bool silence;

	Config() :
		search_depth(90),
		discount(0.95),
		root_seed(42),
		time_per_move(1),
		num_scenarios(500),
		pruning_constant(0),
		xi(0.95),
		sim_len(90),
		default_action(""),
		max_policy_sim_len(90),
		noise(0.1),
		silence(false) {
	}
};

} // namespace despot

#endif
