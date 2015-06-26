
#include "problems/problem_solver.h"

#include "tag.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	model = !options[E_PARAMS_FILE] ?
				new Tag() : new Tag(options[E_PARAMS_FILE].arg);

	return model;
}


int main(int argc, char* argv[]) 
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	/* =========================================
	 * Problem specific default parameter values
	 * =========================================*/
	Globals::config.pruning_constant = 0.01;

	run(argc, argv);

	return 0;
}
