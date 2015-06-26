
#include "problems/problem_solver.h"

#include "pocman.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	model = new FullPocman();

	return model;
}


int main(int argc, char* argv[]) 
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	/* =========================================
	 * Problem specific default parameter values
	 * =========================================*/
	Globals::config.num_scenarios = 100;

	run(argc, argv);

	return 0;
}
