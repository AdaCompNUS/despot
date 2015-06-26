
#include "problems/problem_solver.h"

#include "fvrs.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

	DSPOMDP* model = NULL;

	if (options[E_PARAMS_FILE]) 
	{
		model = new FVRS(options[E_PARAMS_FILE].arg);
	} 
	else 
	{
		int size = 7, number = 8;
		if (options[E_SIZE])
			size = atoi(options[E_SIZE].arg);
		else 
		{
			cerr << "Specify map size using --size option" << endl;
			exit(0);
		}
		if (options[E_NUMBER]) 
		{
			number = atoi(options[E_NUMBER].arg);
		} 
		else 
		{
			cerr << "Specify number of rocks using --number option" << endl;
			exit(0);
		}
		model = new FVRS(size, number);
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
