
#include "problems/problem_solver.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	if (options[E_PARAMS_FILE]) 
	{
		model = new POMDPX(options[E_PARAMS_FILE].arg);
	} 
	else 
	{
		cerr << "ERROR: Specify a POMDPX model file name using -m!" << endl;
		exit(0);
	}

	return model;
}


int main(int argc, char* argv[]) 
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	run(argc, argv);

	return 0;
}
