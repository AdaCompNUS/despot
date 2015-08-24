#include "problem_solver.h"

// option::Arg::Required is a misnomer. The program won't complain if these
// are absent, and required flags must be checked manually.



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

