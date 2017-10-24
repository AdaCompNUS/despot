/*
 * baselinesolver.cpp
 *
 *  Created on: 24 Oct 2017
 *      Author: panpan
 */

#include <despot/core/globals.h>
#include <despot/interface/belief.h>
#include <despot/interface/default_policy.h>
#include <despot/interface/lower_bound.h>
#include <despot/solver/baseline_solver.h>
#include <vector>

using std::vector;

namespace despot {

ScenarioBaselineSolver::ScenarioBaselineSolver(ScenarioLowerBound* lowerbound, Belief* belief):
	Solver(lowerbound->model(),belief){
	lb_solver_=lowerbound;
}

ScenarioBaselineSolver::~ScenarioBaselineSolver() {
}

ValuedAction ScenarioBaselineSolver::Search() {
	RandomStreams streams(Globals::config.num_scenarios,
		Globals::config.search_depth);
	vector<State*> particles = belief_->Sample(Globals::config.num_scenarios);

	ValuedAction va = lb_solver_->Value(particles, streams, history_);

	for (int i = 0; i < particles.size(); i++)
		model_->Free(particles[i]);

	return va;
}



BeliefBaselineSolver::BeliefBaselineSolver(BeliefLowerBound* lowerbound, Belief* belief):
	Solver(lowerbound->model(), belief){
	lb_solver_=lowerbound;
}

BeliefBaselineSolver::~BeliefBaselineSolver() {
}

ValuedAction BeliefBaselineSolver::Search() {
	return lb_solver_->Value(belief_);
}

} /* namespace despot */
