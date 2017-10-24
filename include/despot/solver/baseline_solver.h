/*
 * baselinesolver.h
 *
 *  Created on: 24 Oct 2017
 *      Author: panpan
 */

#ifndef SRC_SOLVER_BASELINESOLVER_H_
#define SRC_SOLVER_BASELINESOLVER_H_

#include <despot/core/solver.h>
#include <despot/interface/lower_bound.h>

namespace despot {

/* =============================================================================
 * ScenarioBaselineSolver class
 * =============================================================================*/
/**
 * Wrapper for using ScenarioLowerBound as base-line solvers.
 */

class ScenarioBaselineSolver: public Solver {
protected:
	ScenarioLowerBound* lb_solver_;
public:
	ScenarioBaselineSolver(ScenarioLowerBound* lowerbound, Belief* belief = NULL);
	virtual ~ScenarioBaselineSolver();

	virtual ValuedAction Search();
};

/* =============================================================================
 * BeliefBaselineSolver class
 * =============================================================================*/
/**
 * Wrapper for using BeliefLowerBound as base-line solvers.
 */

class BeliefBaselineSolver: public Solver {
protected:
	BeliefLowerBound* lb_solver_;
public:
	BeliefBaselineSolver(BeliefLowerBound* lowerbound, Belief* belief = NULL);
	virtual ~BeliefBaselineSolver();

	virtual ValuedAction Search();
};

} /* namespace despot */

#endif /* SRC_SOLVER_BASELINESOLVER_H_ */
