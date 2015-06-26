#include <typeinfo>
#include "despot.h"
#include "aems.h"
#include "pomcp.h"

#include "optionparser.h"

#include "util/seeds.h"
#include "pomdp.h"
#include "client.h"
#include "problems/simulator.h"

enum OptionIndex {
		E_UNKNOWN,
		E_HELP,
	//	E_PROBLEM,
		E_PARAMS_FILE,
		E_DEPTH,
		E_DISCOUNT,
		E_SIZE,
		E_NUMBER,
		E_SEED,
		E_TIMEOUT,
		E_NUMPARTICLES,
		E_PRUNE,
		E_GAP,
		E_SIM_LEN,
		E_SIMULATOR,
		E_MAX_POLICY_SIM_LEN,
		E_DEFAULT_ACTION,
		E_RUNS,
		E_BLBTYPE,
		E_LBTYPE,
		E_BUBTYPE,
		E_UBTYPE,
		E_BELIEF,
		E_KNOWLEDGE,
		E_VERBOSITY,
		E_SILENCE,
		E_SOLVER,
		E_TIME_LIMIT,
		E_NOISE,
		E_SEARCH_SOLVER,
		E_PRIOR,
		E_SERVER,
		E_PORT,
		E_LOG,
};

extern const option::Descriptor usage[];

Solver* InitializeSolver(DSPOMDP* model, string solver_type,option::Option* options);

void optionParse(option::Option* options, int & num_runs, string & simulator_type, 
	string & belief_type, int & time_limit, string &solver_type, bool &search_solver );

void InitializeSimulator(Simulator* &simulator, option::Option* options, DSPOMDP* model,
	 Solver* solver, int num_runs, clock_t main_clock_start, string simulator_type, 
	 string belief_type, int time_limit, string solver_type);

void displayParameters(option::Option* options, DSPOMDP* model);

void getInstance(option::Option* options, Simulator* simulator, string simulator_type, 
	int &num_runs, int &start_run);

	
void runSimulator(DSPOMDP* model, Simulator* simulator, option::Option* options, int num_runs, 
	bool search_solver, Solver * &solver, string simulator_type, clock_t main_clock_start, int start_run);

void printResult(int num_runs, Simulator *simulator, clock_t main_clock_start);

void run(int argc, char* argv[]);

DSPOMDP* InitializeModel(option::Option* options);