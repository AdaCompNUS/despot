#include "problem_solver.h"

// option::Arg::Required is a misnomer. The program won't complain if these
// are absent, and required flags must be checked manually.
const option::Descriptor usage[] =
{ 
	{ E_UNKNOWN, 0, "", "", option::Arg::None, "USAGE: despot [options]\n\nOptions:" },
	{ E_HELP, 0, "", "help", option::Arg::None, "  \t--help\tPrint usage and exit." },
	//{ E_PROBLEM, 0, "q", "problem", option::Arg::Required, "-q <arg>  \t--problem <arg>  \tProblem name." },
	{ E_PARAMS_FILE, 0, "m", "model-params", option::Arg::Required, "-m <arg>  \t--model-params <arg>  \tPath to model-parameters file, if any." },
	{ E_SIZE, 0, "", "size", option::Arg::Required, "  \t--size <arg>  \tSize of a problem (problem specific)." },
	{ E_NUMBER, 0, "", "number", option::Arg::Required, "  \t--number <arg>  \tNumber of elements of a problem (problem specific)." },

	{ E_DEPTH, 0, "d", "depth", option::Arg::Required, "-d <arg>  \t--depth <arg>  \tMaximum depth of search tree (default 90)." },
	{ E_DISCOUNT, 0, "g", "discount", option::Arg::Required, "-g <arg>  \t--discount <arg>  \tDiscount factor (default 0.95)." },
	{ E_TIMEOUT, 0, "t", "timeout", option::Arg::Required, "-t <arg>  \t--timeout <arg>  \tSearch time per move, in seconds (default 1)." },
	{ E_NUMPARTICLES, 0, "n", "nparticles", option::Arg::Required, "-n <arg>  \t--nparticles <arg>  \tNumber of particles (default 500)." },
	{ E_PRUNE, 0, "p", "prune", option::Arg::Required, "-p <arg>  \t--prune <arg>  \tPruning constant (default no pruning)." },
	{ E_GAP, 0, "", "xi", option::Arg::Required, "  \t--xi <arg>  \tGap constant (default to 0.95)." },
	{ E_MAX_POLICY_SIM_LEN, 0, "", "max-policy-simlen", option::Arg::Required, "  \t--max-policy-simlen <arg>  \tDepth to simulate the default policy until. (default 90)." },

	{ E_SEED, 0, "r", "seed", option::Arg::Required, "-r <arg>  \t--seed <arg>  \tRandom number seed (default is random)." },
	{ E_SIM_LEN, 0, "s", "simlen", option::Arg::Required, "-s <arg>  \t--simlen <arg>  \tNumber of steps to simulate. (default 90; 0 = infinite)." },
	{ E_RUNS, 0, "", "runs", option::Arg::Required, "  \t--runs <arg>  \tNumber of runs. (default 1)." }, 
	// { E_SIMULATOR, 0, "", "simulator", option::Arg::Required, "  \t--simulator <arg>  \tUse IPPC server or a POMDP model as the simulator." },
	// { E_DEFAULT_ACTION, 0, "", "default-action", option::Arg::Required, "  \t--default-action <arg>  \tType of default action to use. (default none)." },
	{ E_LBTYPE, 0, "l", "lbtype", option::Arg::Required, "-l <arg>  \t--lbtype <arg>  \tLower bound strategy, if applicable." },
	{ E_BLBTYPE, 0, "", "blbtype", option::Arg::Required, "  \t--blbtype <arg>  \tBase lower bound, if applicable." },
	{ E_UBTYPE, 0, "u", "ubtype", option::Arg::Required, "-u <arg>  \t--ubtype <arg>  \tUpper bound strategy, if applicable." },
	{ E_BUBTYPE, 0, "", "bubtype", option::Arg::Required, "  \t--bubtype <arg>  \tBase upper bound, if applicable." },
	
	{ E_BELIEF, 0, "b", "belief", option::Arg::Required, "-b <arg>  \t--belief <arg>  \tBelief update strategy, if applicable." },
	{ E_NOISE, 0, "", "noise", option::Arg::Required, "  \t--noise <arg>  \tNoise level for transition in POMDPX belief update." },
		
	{ E_VERBOSITY, 0, "v", "verbosity", option::Arg::Required, "-v <arg>  \t--verbosity <arg>  \tVerbosity level." },
	{ E_SILENCE, 0, "", "silence", option::Arg::None, "  \t--silence  \tReduce default output to minimal." },
	{ E_SOLVER, 0, "", "solver", option::Arg::Required, "  \t--solver <arg>  \t" },
	// { E_TIME_LIMIT, 0, "", "time-limit", option::Arg::Required, "  \t--time-limit <arg>  \tTotal amount of time allowed for the program." },
	// { E_SEARCH_SOLVER, 0, "", "search-solver", option::Arg::None, "  \t--search-solver\tUse first few runs to select DESPOT or POMCP as the solver for remaining runs." },
	// { E_PRIOR, 0, "", "prior", option::Arg::Required, "  \t--prior <arg>  \tPOMCP prior." },
	// { E_SERVER, 0, "", "server", option::Arg::Required, "  \t--server <arg>  \tServer address." },
	// { E_PORT, 0, "", "port", option::Arg::Required, "  \t--port <arg>  \tPort number." },
	// { E_LOG, 0, "", "log", option::Arg::Required, "  \t--log <arg>  \tIPPC log file." },
	{ 0, 0, 0, 0, 0, 0 }
};


Solver* InitializeSolver(DSPOMDP* model, string solver_type,option::Option* options) 
{
	Solver* solver = NULL;
	// DESPOT or its default policy
	if (solver_type == "DESPOT" || solver_type == "PLB") //PLB: particle lower bound
	{
		string blbtype = options[E_BLBTYPE] ? options[E_BLBTYPE].arg : "DEFAULT";
		string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
		ScenarioLowerBound* lower_bound = model->CreateScenarioLowerBound(lbtype, blbtype);

		logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

		if (solver_type == "DESPOT") 
		{
			string bubtype = options[E_BUBTYPE] ? options[E_BUBTYPE].arg : "DEFAULT";
			string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
			ScenarioUpperBound* upper_bound = model->CreateScenarioUpperBound(ubtype, bubtype);

			logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

			solver = new DESPOT(model, lower_bound, upper_bound);
		} 
		else solver = lower_bound;
	} // AEMS or its default policy
	else if (solver_type == "AEMS" || solver_type == "BLB") 
	{
		string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
		BeliefLowerBound* lower_bound = static_cast<BeliefMDP*>(model)->CreateBeliefLowerBound(lbtype);

		logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

		if (solver_type == "AEMS") 
		{
			string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
			BeliefUpperBound* upper_bound = static_cast<BeliefMDP*>(model)->CreateBeliefUpperBound(ubtype);

			logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

			solver = new AEMS(model, lower_bound, upper_bound);
		} 
		else solver = lower_bound;
	} // POMCP or DPOMCP
	else if (solver_type == "POMCP" || solver_type == "DPOMCP") 
	{
		string ptype = options[E_PRIOR] ? options[E_PRIOR].arg : "DEFAULT";
		POMCPPrior* prior = model->CreatePOMCPPrior(ptype);

		logi << "Created POMCP prior " << typeid(*prior).name() << endl;

		if (options[E_PRUNE]) 
		{
			prior->exploration_constant(Globals::config.pruning_constant);
		}

		if (solver_type == "POMCP") solver = new POMCP(model, prior);
		else solver = new DPOMCP(model, prior);
	} 
	else 
	{ // Unsupported solver
		cerr << "ERROR: Unsupported solver type: " << solver_type << endl;
		exit(1);
	}
	return solver;
}


void optionParse(option::Option* options, int & num_runs, string & simulator_type, string & belief_type, int & time_limit, string &solver_type, bool &search_solver )
{
	if (options[E_SILENCE]) Globals::config.silence = true;
	
	if (options[E_DEPTH]) Globals::config.search_depth = atoi(options[E_DEPTH].arg);
	
	if (options[E_DISCOUNT]) Globals::config.discount = atof(options[E_DISCOUNT].arg);

	if (options[E_SEED]) Globals::config.root_seed = atoi(options[E_SEED].arg);
	else 
	{ // last 9 digits of current time in milli second
		long millis = (long) get_time_second() * 1000;
		long range = (long) pow((double) 10, (int) 9);
		Globals::config.root_seed = (unsigned int) (millis
			- (millis / range) * range);
	}

	if (options[E_TIMEOUT]) Globals::config.time_per_move = atof(options[E_TIMEOUT].arg);

	if (options[E_NUMPARTICLES]) Globals::config.num_scenarios = atoi(options[E_NUMPARTICLES].arg);

	if (options[E_PRUNE]) Globals::config.pruning_constant = atof(options[E_PRUNE].arg);
	

	if (options[E_GAP]) Globals::config.xi = atof(options[E_GAP].arg);

	if (options[E_SIM_LEN]) Globals::config.sim_len = atoi(options[E_SIM_LEN].arg);

	if (options[E_SIMULATOR])simulator_type = options[E_SIMULATOR].arg;

	if (options[E_MAX_POLICY_SIM_LEN]) 
		Globals::config.max_policy_sim_len = atoi(
			options[E_MAX_POLICY_SIM_LEN].arg);

	if (options[E_DEFAULT_ACTION]) Globals::config.default_action = options[E_DEFAULT_ACTION].arg;

	if (options[E_RUNS]) num_runs = atoi(options[E_RUNS].arg);

	if (options[E_BELIEF]) belief_type = options[E_BELIEF].arg;

	if (options[E_TIME_LIMIT]) time_limit = atoi(options[E_TIME_LIMIT].arg);

	if (options[E_NOISE]) Globals::config.noise = atof(options[E_NOISE].arg);

	search_solver = options[E_SEARCH_SOLVER];

	if (options[E_SOLVER]) solver_type = options[E_SOLVER].arg;

	int verbosity = 0;
	if (options[E_VERBOSITY]) verbosity = atoi(options[E_VERBOSITY].arg);
	logging::level(verbosity);
}


void InitializeSimulator(Simulator* &simulator, option::Option* options, DSPOMDP* model,
	 Solver* solver, int num_runs, clock_t main_clock_start, string simulator_type, 
	 string belief_type, int time_limit, string solver_type)
{
	/*
	if (simulator_type == "ippc") 
	{
		if (!options[E_SERVER]) 
		{
			cerr << "ERROR: Specify the server name using the --server option!"<< endl;
			exit(0);
		}

		if (!options[E_PORT]) 
		{
			cerr << "ERROR: Specify the port number using the --port option!"<< endl;
			exit(0);
		}

		if (!options[E_LOG]) 
		{
			cerr << "ERROR: Specify the log file the --log option!" << endl;
			exit(0);
		}

		string log_file = options[E_LOG].arg;
		IPPCLog log(log_file);
		log.Save(); // Initialize log file
		cout << "Time: elapsed_since_start / total_remaining = "
			<< (IPPCLog::curr_inst_start_time - IPPCLog::start_time) << " / "
			<< (24 * 3600 - (IPPCLog::curr_inst_start_time - IPPCLog::start_time))
			<< endl;

		string server = options[E_SERVER].arg;
		string port = options[E_PORT].arg;

		simulator = new IPPCSimulator(model, belief_type, solver,
			main_clock_start, server, port, log_file, &cout);
	} 
	else */
	{
		if (time_limit != -1) 
		{
			simulator = new POMDPSimulator(model, belief_type, solver,
				main_clock_start, &cout,
				IPPCLog::curr_inst_start_time + time_limit,
				num_runs * Globals::config.sim_len);
		} 
		else 
		{
			simulator = new POMDPSimulator(model, belief_type, solver,
				main_clock_start, &cout);
		}
	}
}


void displayParameters(option::Option* options, DSPOMDP* model)
{

	string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
	string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
	default_out << "Model = " << typeid(*model).name() << endl
		<< "Random root seed = " << Globals::config.root_seed << endl
		<< "Search depth = " << Globals::config.search_depth << endl
		<< "Discount = " << Globals::config.discount << endl
		<< "Simulation steps = " << Globals::config.sim_len << endl
		<< "Number of scenarios = " << Globals::config.num_scenarios << endl
		<< "Search time per step = " << Globals::config.time_per_move << endl
		<< "Regularization constant = " << Globals::config.pruning_constant << endl
		<< "Lower bound = " << lbtype << endl
		<< "Upper bound = " << ubtype << endl
		<< "Policy simulation depth = " << Globals::config.max_policy_sim_len << endl
		<< "Target gap ratio = " << Globals::config.xi << endl;
		// << "Solver = " << typeid(*solver).name() << endl << endl;
}

/*
void getInstance(option::Option* options, Simulator* simulator, string simulator_type, 
	int &num_runs, int &start_run)
{	
	if (simulator_type == "ippc") 
	{
		string input_file(options[E_PARAMS_FILE].arg);
		int seconddot = input_file.find_last_of('.');
		int firstdot = input_file.substr(0, seconddot).find_last_of('/');
		string instance = input_file.substr(firstdot + 1, seconddot - firstdot - 1);

		num_runs = simulator->Handshake(instance);
		start_run = static_cast<IPPCSimulator*>(simulator)->GetNumCompletedRuns(instance);
		if (num_runs == 0) 
		{
			cout << "Finished running instance. Exit." << endl;
			exit(0);
		}
		cout << num_runs << " runs to be completed in "
			<< IPPCLog::curr_inst_budget << " seconds" << endl;
	}
}
*/
	
void runSimulator(DSPOMDP* model, Simulator* simulator, option::Option* options, int num_runs, 
	bool search_solver, Solver * &solver, string simulator_type, clock_t main_clock_start, int start_run)
{
	// Run num_runs simulations
	vector<double> round_rewards(num_runs);
	for (int round = start_run; round < start_run + num_runs; round++) {
		default_out << endl
			<< "####################################### Round " << round
			<< " #######################################" << endl;

		if (search_solver) {
			if (round == 0) {
				solver = InitializeSolver(model, "DESPOT", options);
				default_out << "Solver: " << typeid(*solver).name() << endl;

				simulator->solver(solver);
			} else if (round == 5) {
				solver = InitializeSolver(model, "POMCP", options);
				default_out << "Solver: " << typeid(*solver).name() << endl;

				simulator->solver(solver);
			} else if (round == 10) {
				double sum1 = 0,
							 sum2 = 0;
				for (int i = 0; i < 5; i ++)
					sum1 += round_rewards[i];
				for (int i = 5; i < 10; i ++)
					sum2 += round_rewards[i];
				if (sum1 < sum2)
					solver = InitializeSolver(model, "POMCP", options);
				else
					solver = InitializeSolver(model, "DESPOT", options);
				default_out << "Solver: " << typeid(*solver).name() << " DESPOT:" << sum1 << " POMCP:" << sum2 << endl;
			}

			simulator->solver(solver);
		}

		simulator->InitRound();

		for (int i = 0; i < Globals::config.sim_len; i++) {
			default_out << "-----------------------------------Round " << round
				<< " Step " << i << "-----------------------------------"
				<< endl;
			double step_start_t = get_time_second();

			bool terminal = simulator->RunStep();

			if (terminal)
				break;

			double step_end_t = get_time_second();
			logi << "[main] Time for step: actual / allocated = "
				<< (step_end_t - step_start_t) << " / "
				<< IPPCLog::allocated_time << endl;
			simulator->UpdateTimePerMove(step_end_t - step_start_t);
			logi << "[main] Time per move set to "
				<< Globals::config.time_per_move << endl;
			logi << "[main] Plan time ratio set to " << IPPCLog::plan_time_ratio
				<< endl;
			default_out << endl;
		}

		default_out << "Simulation terminated in " << simulator->step() << " steps"
			<< endl;
		double round_reward = simulator->EndRound();
		round_rewards[round] = round_reward;
	}

	if (simulator_type == "ippc" && num_runs != 30) {
		cout << "Exit without receiving reward." << endl
			<< "Total time: Real / CPU = "
			<< (get_time_second() - IPPCLog::curr_inst_start_time) << " / "
			<< (double(clock() - main_clock_start) / CLOCKS_PER_SEC) << "s"
			<< endl;
		exit(0);
	}
}

void printResult(int num_runs, Simulator *simulator, clock_t main_clock_start)
{

	cout << "\nCompleted " << num_runs << " run(s)." << endl;
	cout << "Average total discounted reward (stderr) = "
		<< simulator->AverageDiscountedRoundReward() <<
		" (" << simulator->StderrDiscountedRoundReward() << ")" << endl;
	cout << "Average total undiscounted reward (stderr) = "
		<< simulator->AverageUndiscountedRoundReward() <<
		" (" << simulator->StderrUndiscountedRoundReward() << ")" << endl;
	cout << "Total time: Real / CPU = "
		<< (get_time_second() - IPPCLog::curr_inst_start_time) << " / "
		<< (double(clock() - main_clock_start) / CLOCKS_PER_SEC) << "s" << endl;
}

