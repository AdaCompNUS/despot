/*
 * Evaluator.h
 *
 *  Created on: 5 Oct 2017
 *      Author: panpan
 */

#ifndef INCLUDE_DESPOT_EVALUATOR_H_
#define INCLUDE_DESPOT_EVALUATOR_H_

#include <despot/planner.h>

namespace despot {

class Evaluator: public Planner {
public:
	Evaluator(string lower_bounds_str = "TRIVIAL",
			string base_lower_bounds_str = "TRIVIAL", string upper_bounds_str =
					"TRIVIAL", string base_upper_bounds_str = "TRIVIAL");
	virtual ~Evaluator();

	/**
	 * Run and evaluate POMDP planning for a given number of rounds
	 */
	int runEvaluation(int argc, char* argv[]);

	/**
	 * Loop the planning process for a given number of rounds
	 */
	void EvaluationLoop(DSPOMDP *model, World* world, Belief* belief,
			std::string belief_type, Solver *&solver, Logger *logger,
			option::Option *options, clock_t main_clock_start, int num_runs,
			int start_run);

};

} /* namespace despot */

#endif /* INCLUDE_DESPOT_EVALUATOR_H_ */
