
#include "problems/problem_solver.h"

#include "simple_rock_sample.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	model = new SimpleRockSample();

	return model;
}


int main(int argc, char* argv[]) 
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	run(argc, argv);

	return 0;
}
