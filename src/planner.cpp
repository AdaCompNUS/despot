/*
 * Planner.cpp
 *
 *  Created on: 5 Oct 2017
 *      Author: panpan
 */

#include <despot/core/solver.h>
#include <despot/interface/belief.h>
#include <despot/interface/world.h>
#include <despot/logger.h>
#include <despot/planner.h>

namespace despot {

Planner::Planner(string lower_bounds_str,
		string base_lower_bounds_str, string upper_bounds_str, string base_upper_bounds_str)
		:PlannerBase(lower_bounds_str, base_lower_bounds_str, upper_bounds_str, base_upper_bounds_str){
	step_=0;
	round_=0;
}

Planner::~Planner() {
}


bool Planner::RunStep(Solver* solver, World* world, Logger* logger) {

	logger->CheckTargetTime();

	double step_start_t = get_time_second();

	double start_t = get_time_second();
	ACT_TYPE action = solver->Search().action;
	double end_t = get_time_second();
	double search_time = (end_t - start_t);
	logi << "[RunStep] Time spent in " << typeid(*solver).name()
			<< "::Search(): " << search_time << endl;

	OBS_TYPE obs;
	start_t = get_time_second();
	bool terminal = world->ExecuteAction(action, obs);
	end_t = get_time_second();
	double execute_time = (end_t - start_t);
	logi << "[RunStep] Time spent in ExecuteAction(): " << execute_time << endl;

	start_t = get_time_second();
	solver->BeliefUpdate(action, obs);
	end_t = get_time_second();
	double update_time = (end_t - start_t);
	logi << "[RunStep] Time spent in Update(): " << update_time << endl;

	return logger->SummarizeStep(step_++, round_, terminal, action, obs,
			step_start_t);
}

void Planner::PlanningLoop(Solver*& solver, World* world, Logger* logger) {
	for (int i = 0; i < Globals::config.sim_len; i++) {
		bool terminal = RunStep(solver, world, logger);
		if (terminal)
			break;
	}
}

int Planner::RunPlanning(int argc, char *argv[]) {

	/* =========================
	 * initialize parameters
	 * =========================*/
	string solver_type = ChooseSolver(); //"DESPOT";
	bool search_solver;
	int num_runs = 1;
	string world_type = "pomdp";
	string belief_type = "DEFAULT";
	int time_limit = -1;

	option::Option *options = InitializeParamers(argc, argv, solver_type,
			search_solver, num_runs, world_type, belief_type, time_limit);
	if(options==NULL)
		return 0;
	clock_t main_clock_start = clock();

	/* =========================
	 * initialize model
	 * =========================*/
	DSPOMDP *model = InitializeModel(options);
	assert(model != NULL);

	/* =========================
	 * initialize world
	 * =========================*/
	World *world = InitializeWorld(world_type, model, options);
	assert(world != NULL);

	/* =========================
	 * initialize belief
	 * =========================*/
	Belief* belief = model->InitialBelief(world->GetCurrentState(), belief_type);
	assert(belief != NULL);

	/* =========================
	 * initialize solver
	 * =========================*/
	Solver *solver = InitializeSolver(model, belief, solver_type, options);

	/* =========================
	 * initialize logger
	 * =========================*/
	Logger *logger = NULL;
	InitializeLogger(logger, options, model, belief, solver, num_runs,
			main_clock_start, world, world_type, time_limit, solver_type);
	//world->world_seed(world_seed);

	/* =========================
	 * Display parameters
	 * =========================*/
	DisplayParameters(options, model);

	/* =========================
	 * run planning
	 * =========================*/
	logger->InitRound(world->GetCurrentState());
	round_=0; step_=0;
	PlanningLoop(solver, world, logger);
	logger->EndRound();

	PrintResult(1, logger, main_clock_start);

	return 0;
}

} /* namespace despot */
