/*
 * Planner.h
 *
 *  Created on: 5 Oct 2017
 *      Author: panpan
 */

#ifndef INCLUDE_DESPOT_PLANNER_H_
#define INCLUDE_DESPOT_PLANNER_H_

#include <despot/plannerbase.h>

namespace despot {

class Planner: public PlannerBase {
protected:
	int step_;
	int round_;
public:
	Planner(string lower_bounds_str = "TRIVIAL",
			string base_lower_bounds_str = "TRIVIAL", string upper_bounds_str =
					"TRIVIAL", string base_upper_bounds_str = "TRIVIAL");

	virtual ~Planner();

	/**
	 * Perform one search-execute-update step
	 */
	virtual bool RunStep(Solver* solver, World* world, Logger* logger);

	/**
	 * Run POMDP planning till terminal reached or time out
	 */
	virtual int RunPlanning(int argc, char* argv[]);

	/**
	 * Loop the search-execute-update process for a given number of steps
	 * Overwrite this function to customize your planning pipeline
	 */
	virtual void PlanningLoop(Solver*& solver, World* world, Logger* logger);

	/**
	 * Run and evaluate POMDP planning for a given number of rounds
	 */
	virtual int RunEvaluation(int argc, char* argv[]);

	/**
	 * Evaluate the planner by repeating a test problem for multiple trials
	 * Overwrite this function to customize your evaluation pipeline
	 */
	virtual void EvaluationLoop(DSPOMDP *model, World* world, Belief* belief,
			std::string belief_type, Solver *&solver, Logger *logger,
			option::Option *options, clock_t main_clock_start, int num_runs,
			int start_run);

};

} /* namespace despot */

#endif /* INCLUDE_DESPOT_PLANNER_H_ */
