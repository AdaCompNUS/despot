/*
 * Planner.h
 *
 *  Created on: 5 Oct 2017
 *      Author: panpan
 */

#ifndef INCLUDE_DESPOT_PLANNER_H_
#define INCLUDE_DESPOT_PLANNER_H_

#include <despot/initializer.h>

namespace despot {

class Planner: public Initializer {
public:
	Planner(string lower_bounds_str = "TRIVIAL",
			string base_lower_bounds_str = "TRIVIAL", string upper_bounds_str =
					"TRIVIAL", string base_upper_bounds_str = "TRIVIAL");

	virtual ~Planner();

	/**
	 * Perform one search-execute-update step
	 */
	bool RunStep(int step, int round, Solver* solver, World* world,
			Logger* logger);

	/**
	 * Run POMDP planning for one round
	 */
	int runPlanning(int argc, char* argv[]);

	/**
	 * Loop the search-execute-update process for a given number of steps
	 */
	void PlanningLoop(int round, Solver*& solver, World* world,
			Logger* logger);

};

} /* namespace despot */

#endif /* INCLUDE_DESPOT_PLANNER_H_ */
