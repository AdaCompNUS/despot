#include <despot/simple_tui.h>

using namespace std;

namespace despot {

void disableBufferedIO(void) {
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  setbuf(stderr, NULL);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

option::Descriptor* buildUsage(string lower_bounds_str, 
		string base_lower_bounds_str, 
		string upper_bounds_str, 
		string base_upper_bounds_str)
{
	static string lb_str = "-l <arg>  \t--lbtype <arg>  \tLower bound strategy (" + lower_bounds_str + ").";
	static string blb_str = "  \t--blbtype <arg>  \tBase lower bound (" + base_lower_bounds_str + ").";
	static string ub_str = "-u <arg>  \t--ubtype <arg>  \tUpper bound strategy (" + upper_bounds_str + ").";
	static string bub_str = "  \t--bubtype <arg>  \tBase upper bound (" + base_upper_bounds_str + ").";
	// option::Arg::Required is a misnomer. The program won't complain if these
	// are absent, and required flags must be checked manually.
	static option::Descriptor usage[] = {
		{ E_HELP, 0, "", "help", option::Arg::None,
			"  \t--help\tPrint usage and exit." },
		{ E_PARAMS_FILE, 0, "m", "model-params", option::Arg::Required,
			"-m <arg>  \t--model-params <arg>  \tPath to model-parameters file, if "
			"any." },
		{ E_SIZE, 0, "", "size", option::Arg::Required,
			"  \t--size <arg>  \tSize of a problem (problem specific)." },
		{ E_NUMBER, 0, "", "number", option::Arg::Required,
			"  \t--number <arg>  \tNumber of elements of a problem (problem "
			"specific)." },
		{ E_DEPTH, 0, "d", "depth", option::Arg::Required,
			"-d <arg>  \t--depth <arg>  \tMaximum depth of search tree (default 90)." },
		{ E_DISCOUNT, 0, "g", "discount", option::Arg::Required,
			"-g <arg>  \t--discount <arg>  \tDiscount factor (default 0.95)." },
		{ E_TIMEOUT, 0, "t", "timeout", option::Arg::Required,
			"-t <arg>  \t--timeout <arg>  \tSearch time per move, in seconds (default "
			"1)." },
		{ E_NUMPARTICLES, 0, "n", "nparticles", option::Arg::Required,
			"-n <arg>  \t--nparticles <arg>  \tNumber of particles (default 500)." },
		{ E_PRUNE, 0, "p", "prune", option::Arg::Required,
			"-p <arg>  \t--prune <arg>  \tPruning constant (default no pruning)." },
		{ E_GAP, 0, "", "xi", option::Arg::Required,
			"  \t--xi <arg>  \tGap constant (default to 0.95)." },
		{ E_MAX_POLICY_SIM_LEN, 0, "", "max-policy-simlen", option::Arg::Required,
			"  \t--max-policy-simlen <arg>  \tDepth to simulate the default policy "
			"until. (default 90)." },

		{ E_SEED, 0, "r", "seed", option::Arg::Required,
			"-r <arg>  \t--seed <arg>  \tRandom number seed (default is random)." },
		{ E_SIM_LEN, 0, "s", "simlen", option::Arg::Required,
			"-s <arg>  \t--simlen <arg>  \tNumber of steps to simulate. (default 90; 0 "
			"= infinite)." },
		{ E_RUNS, 0, "", "runs", option::Arg::Required,
			"  \t--runs <arg>  \tNumber of runs. (default 1)." },
		{ E_LBTYPE, 0, "l", "lbtype", option::Arg::Required, lb_str.c_str()},
		{ E_BLBTYPE, 0, "", "blbtype", option::Arg::Required, blb_str.c_str()},	
		{ E_UBTYPE, 0, "u", "ubtype", option::Arg::Required, ub_str.c_str()},
		{ E_BUBTYPE, 0, "", "bubtype", option::Arg::Required, bub_str.c_str()},
		{ E_BELIEF, 0, "b", "belief", option::Arg::Required,
			"-b <arg>  \t--belief <arg>  \tBelief update strategy, if applicable." },
		{ E_NOISE, 0, "", "noise", option::Arg::Required,
			"  \t--noise <arg>  \tNoise level for transition in POMDPX belief "
			"update." },

		{ E_VERBOSITY, 0, "v", "verbosity", option::Arg::Required,
			"-v <arg>  \t--verbosity <arg>  \tVerbosity level." },
		{ E_SILENCE, 0, "", "silence", option::Arg::None,
			"  \t--silence  \tReduce default output to minimal." },
		{ E_SOLVER, 0, "", "solver", option::Arg::Required,
			"  \t--solver <arg>  \t" },
		{ E_PRIOR, 0, "", "prior", option::Arg::Required, 
			"  \t--prior <arg>  \tPOMCP prior." },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return usage;
}

SimpleTUI::SimpleTUI(string lower_bounds_str,
		string base_lower_bounds_str,
		string upper_bounds_str,
		string base_upper_bounds_str) {
	usage = buildUsage(lower_bounds_str, base_lower_bounds_str,
			upper_bounds_str, base_upper_bounds_str);
}

SimpleTUI::~SimpleTUI() {}

Solver *SimpleTUI::InitializeSolver(DSPOMDP *model, string solver_type,
                                    option::Option *options) {
  Solver *solver = NULL;
  // DESPOT or its default policy
  if (solver_type == "DESPOT" ||
      solver_type == "PLB") // PLB: particle lower bound
  {
    string blbtype = options[E_BLBTYPE] ? options[E_BLBTYPE].arg : "DEFAULT";
    string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
    ScenarioLowerBound *lower_bound =
        model->CreateScenarioLowerBound(lbtype, blbtype);

    logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

    if (solver_type == "DESPOT") {
      string bubtype = options[E_BUBTYPE] ? options[E_BUBTYPE].arg : "DEFAULT";
      string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
      ScenarioUpperBound *upper_bound =
          model->CreateScenarioUpperBound(ubtype, bubtype);

      logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

      solver = new DESPOT(model, lower_bound, upper_bound);
    } else
      solver = lower_bound;
  } // AEMS or its default policy
  else if (solver_type == "AEMS" || solver_type == "BLB") {
    string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
    BeliefLowerBound *lower_bound =
        static_cast<BeliefMDP *>(model)->CreateBeliefLowerBound(lbtype);

    logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

    if (solver_type == "AEMS") {
      string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
      BeliefUpperBound *upper_bound =
          static_cast<BeliefMDP *>(model)->CreateBeliefUpperBound(ubtype);

      logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

      solver = new AEMS(model, lower_bound, upper_bound);
    } else
      solver = lower_bound;
  } // POMCP or DPOMCP
  else if (solver_type == "POMCP" || solver_type == "DPOMCP") {
    string ptype = options[E_PRIOR] ? options[E_PRIOR].arg : "DEFAULT";
    POMCPPrior *prior = model->CreatePOMCPPrior(ptype);

    logi << "Created POMCP prior " << typeid(*prior).name() << endl;

    if (options[E_PRUNE]) {
      prior->exploration_constant(Globals::config.pruning_constant);
    }

    if (solver_type == "POMCP")
      solver = new POMCP(model, prior);
    else
      solver = new DPOMCP(model, prior);
  } else { // Unsupported solver
    cerr << "ERROR: Unsupported solver type: " << solver_type << endl;
    exit(1);
  }
  return solver;
}

void SimpleTUI::OptionParse(option::Option *options, int &num_runs,
                            string &simulator_type, string &belief_type,
                            int &time_limit, string &solver_type,
                            bool &search_solver) {
  if (options[E_SILENCE])
    Globals::config.silence = true;

  if (options[E_DEPTH])
    Globals::config.search_depth = atoi(options[E_DEPTH].arg);

  if (options[E_DISCOUNT])
    Globals::config.discount = atof(options[E_DISCOUNT].arg);

  if (options[E_SEED])
    Globals::config.root_seed = atoi(options[E_SEED].arg);
  else { // last 9 digits of current time in milli second
    long millis = (long)get_time_second() * 1000;
    long range = (long)pow((double)10, (int)9);
    Globals::config.root_seed =
        (unsigned int)(millis - (millis / range) * range);
  }

  if (options[E_TIMEOUT])
    Globals::config.time_per_move = atof(options[E_TIMEOUT].arg);

  if (options[E_NUMPARTICLES])
    Globals::config.num_scenarios = atoi(options[E_NUMPARTICLES].arg);

  if (options[E_PRUNE])
    Globals::config.pruning_constant = atof(options[E_PRUNE].arg);

  if (options[E_GAP])
    Globals::config.xi = atof(options[E_GAP].arg);

  if (options[E_SIM_LEN])
    Globals::config.sim_len = atoi(options[E_SIM_LEN].arg);

  if (options[E_EVALUATOR])
    simulator_type = options[E_EVALUATOR].arg;

  if (options[E_MAX_POLICY_SIM_LEN])
    Globals::config.max_policy_sim_len =
        atoi(options[E_MAX_POLICY_SIM_LEN].arg);

  if (options[E_DEFAULT_ACTION])
    Globals::config.default_action = options[E_DEFAULT_ACTION].arg;

  if (options[E_RUNS])
    num_runs = atoi(options[E_RUNS].arg);

  if (options[E_BELIEF])
    belief_type = options[E_BELIEF].arg;

  if (options[E_TIME_LIMIT])
    time_limit = atoi(options[E_TIME_LIMIT].arg);

  if (options[E_NOISE])
    Globals::config.noise = atof(options[E_NOISE].arg);

  search_solver = options[E_SEARCH_SOLVER];

  if (options[E_SOLVER])
    solver_type = options[E_SOLVER].arg;

  int verbosity = 0;
  if (options[E_VERBOSITY])
    verbosity = atoi(options[E_VERBOSITY].arg);
  logging::level(verbosity);
}

void SimpleTUI::InitializeEvaluator(Evaluator *&simulator,
                                    option::Option *options, DSPOMDP *model,
                                    Solver *solver, int num_runs,
                                    clock_t main_clock_start,
                                    string simulator_type, string belief_type,
                                    int time_limit, string solver_type) {

  if (time_limit != -1) {
    simulator =
        new POMDPEvaluator(model, belief_type, solver, main_clock_start, &cout,
                           EvalLog::curr_inst_start_time + time_limit,
                           num_runs * Globals::config.sim_len);
  } else {
    simulator =
        new POMDPEvaluator(model, belief_type, solver, main_clock_start, &cout);
  }
}

void SimpleTUI::DisplayParameters(option::Option *options, DSPOMDP *model) {

  string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
  string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
  default_out << "Model = " << typeid(*model).name() << endl
              << "Random root seed = " << Globals::config.root_seed << endl
              << "Search depth = " << Globals::config.search_depth << endl
              << "Discount = " << Globals::config.discount << endl
              << "Simulation steps = " << Globals::config.sim_len << endl
              << "Number of scenarios = " << Globals::config.num_scenarios
              << endl
              << "Search time per step = " << Globals::config.time_per_move
              << endl
              << "Regularization constant = "
              << Globals::config.pruning_constant << endl
              << "Lower bound = " << lbtype << endl
              << "Upper bound = " << ubtype << endl
              << "Policy simulation depth = "
              << Globals::config.max_policy_sim_len << endl
              << "Target gap ratio = " << Globals::config.xi << endl;
  // << "Solver = " << typeid(*solver).name() << endl << endl;
}

void SimpleTUI::RunEvaluator(DSPOMDP *model, Evaluator *simulator,
                             option::Option *options, int num_runs,
                             bool search_solver, Solver *&solver,
                             string simulator_type, clock_t main_clock_start,
                             int start_run) {
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
        double sum1 = 0, sum2 = 0;
        for (int i = 0; i < 5; i++)
          sum1 += round_rewards[i];
        for (int i = 5; i < 10; i++)
          sum2 += round_rewards[i];
        if (sum1 < sum2)
          solver = InitializeSolver(model, "POMCP", options);
        else
          solver = InitializeSolver(model, "DESPOT", options);
        default_out << "Solver: " << typeid(*solver).name()
                    << " DESPOT:" << sum1 << " POMCP:" << sum2 << endl;
      }

      simulator->solver(solver);
    }

    simulator->InitRound();

    for (int i = 0; i < Globals::config.sim_len; i++) {
      /*
      default_out << "-----------------------------------Round " << round
                  << " Step " << i << "-----------------------------------"
                  << endl;*/
      double step_start_t = get_time_second();

      bool terminal = simulator->RunStep(i, round);

      if (terminal)
        break;

      double step_end_t = get_time_second();
      logi << "[main] Time for step: actual / allocated = "
           << (step_end_t - step_start_t) << " / " << EvalLog::allocated_time
           << endl;
      simulator->UpdateTimePerMove(step_end_t - step_start_t);
      logi << "[main] Time per move set to " << Globals::config.time_per_move
           << endl;
      logi << "[main] Plan time ratio set to " << EvalLog::plan_time_ratio
           << endl;
    //  default_out << endl;
    }

    default_out << "Simulation terminated in " << simulator->step() << " steps"
                << endl;
    double round_reward = simulator->EndRound();
    round_rewards[round] = round_reward;
  }

  if (simulator_type == "ippc" && num_runs != 30) {
    cout << "Exit without receiving reward." << endl
         << "Total time: Real / CPU = "
         << (get_time_second() - EvalLog::curr_inst_start_time) << " / "
         << (double(clock() - main_clock_start) / CLOCKS_PER_SEC) << "s"
         << endl;
    exit(0);
  }
}

void SimpleTUI::PrintResult(int num_runs, Evaluator *simulator,
                            clock_t main_clock_start) {

  cout << "\nCompleted " << num_runs << " run(s)." << endl;
  cout << "Average total discounted reward (stderr) = "
       << simulator->AverageDiscountedRoundReward() << " ("
       << simulator->StderrDiscountedRoundReward() << ")" << endl;
  cout << "Average total undiscounted reward (stderr) = "
       << simulator->AverageUndiscountedRoundReward() << " ("
       << simulator->StderrUndiscountedRoundReward() << ")" << endl;
  cout << "Total time: Real / CPU = "
       << (get_time_second() - EvalLog::curr_inst_start_time) << " / "
       << (double(clock() - main_clock_start) / CLOCKS_PER_SEC) << "s" << endl;
}

int SimpleTUI::run(int argc, char *argv[]) {

  clock_t main_clock_start = clock();
  EvalLog::curr_inst_start_time = get_time_second();

  const char *program = (argc > 0) ? argv[0] : "despot";

  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0] if present

  option::Stats stats(usage, argc, argv);
  option::Option *options = new option::Option[stats.options_max];
  option::Option *buffer = new option::Option[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);

  string solver_type = "DESPOT";
  bool search_solver;

  /* =========================
   * Parse required parameters
   * =========================*/
  int num_runs = 1;
  string simulator_type = "pomdp";
  string belief_type = "DEFAULT";
  int time_limit = -1;

  /* =========================================
   * Problem specific default parameter values
*=========================================*/
  InitializeDefaultParameters();

  /* =========================
   * Parse optional parameters
   * =========================*/
  if (options[E_HELP]) {
    cout << "Usage: " << program << " [options]" << endl;
    option::printUsage(std::cout, usage);
    return 0;
  }
  OptionParse(options, num_runs, simulator_type, belief_type, time_limit,
              solver_type, search_solver);

  /* =========================
   * Global random generator
   * =========================*/
  Seeds::root_seed(Globals::config.root_seed);
  unsigned world_seed = Seeds::Next();
  unsigned seed = Seeds::Next();
  Random::RANDOM = Random(seed);

  /* =========================
   * initialize model
   * =========================*/
  DSPOMDP *model = InitializeModel(options);

  /* =========================
   * initialize solver
   * =========================*/
  Solver *solver = InitializeSolver(model, solver_type, options);
  assert(solver != NULL);

  /* =========================
   * initialize simulator
   * =========================*/
  Evaluator *simulator = NULL;
  InitializeEvaluator(simulator, options, model, solver, num_runs,
                      main_clock_start, simulator_type, belief_type, time_limit,
                      solver_type);
  simulator->world_seed(world_seed);

  int start_run = 0;

  /* =========================
   * Display parameters
   * =========================*/
  DisplayParameters(options, model);

  /* =========================
   * run simulator
   * =========================*/
  RunEvaluator(model, simulator, options, num_runs, search_solver, solver,
               simulator_type, main_clock_start, start_run);

  simulator->End();

  PrintResult(num_runs, simulator, main_clock_start);

  return 0;
}

} // namespace despot
