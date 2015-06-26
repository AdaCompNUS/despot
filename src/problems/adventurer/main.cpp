
#include "problems/problem_solver.h"

#include "adventurer.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	int num_goals = options[E_SIZE] ? atoi(options[E_SIZE].arg) : 50;
	model =	!options[E_PARAMS_FILE] ?
			new Adventurer(num_goals) : new Adventurer(options[E_PARAMS_FILE].arg);

	return model;
}


int main(int argc, char* argv[]) 
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	/* =========================================
	 * Problem specific default parameter values
	 * =========================================*/
	Globals::config.search_depth = 5;
	Globals::config.sim_len = 5;

	run(argc, argv);

	return 0;
}
