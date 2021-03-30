#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace despot {

struct Config {
	double time_per_move;  // CPU time available to construct the search tree
	int sim_len; // The number of simulation steps for each episode.
	int num_scenarios; // The number of scenarios usedto generate the DESPOT tree
	int search_depth; // The maximum depth of the search tree
	int max_policy_sim_len; // Maximum number of steps for simulating the default policy (rollout). Note that the depth of rollouts won't exceed the maximum search depth.
	double discount; // The discount factor
	double pruning_constant; // The pruning constant attached to each node for regularization purpose
	double xi; // xi * gap(root) is the target uncertainty at the root.
	unsigned int root_seed;
	std::string default_action;
	double noise;
	bool silence; // toggle logging

	Config() :
		time_per_move(1),
		sim_len(90),
		num_scenarios(500),
		search_depth(90),
		max_policy_sim_len(90),
		discount(0.95),
		pruning_constant(0),
		xi(0.95),
		root_seed(42),
		default_action(""),
		noise(0.1),
		silence(false) {
	}
};

} // namespace despot

#endif
