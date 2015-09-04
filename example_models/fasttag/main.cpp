#include "problem_solver.h"
#include "fasttag.h"

using namespace std;


DSPOMDP* InitializeModel(option::Option* options) 
{

  DSPOMDP* model = NULL;

  model = !options[E_PARAMS_FILE] ?
        new FastTag() : new FastTag(options[E_PARAMS_FILE].arg);

  return model;
}

void run(int argc, char* argv[])
{

  clock_t main_clock_start = clock();
  IPPCLog::curr_inst_start_time = get_time_second();

  option::Stats stats(usage, argc, argv);
  option::Option* options = new option::Option[stats.options_max];
  option::Option* buffer = new option::Option[stats.buffer_max];
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


  /* =========================
   * Parse optional parameters
   * =========================*/
  optionParse(options, num_runs,simulator_type, belief_type, time_limit, solver_type, 
    search_solver);

  /* =========================
   * Global random generator
   * =========================*/
  Seeds::root_seed(Globals::config.root_seed);
  unsigned world_seed = Seeds::Next();
  unsigned seed = Seeds::Next();
  Random::RANDOM = Random(seed);

  if (options[E_HELP]) 
  {
    option::printUsage(std::cout, usage);
    return;
  }

  /* =========================
   * initialize model
   * =========================*/
  DSPOMDP* model = InitializeModel(options);

  /* =========================
   * initialize solver
   * =========================*/
  Solver* solver = InitializeSolver(model, solver_type, options);
  assert(solver != NULL);

  /* =========================
   * initialize simulator
   * =========================*/
  Simulator* simulator = NULL;
  InitializeSimulator(simulator, options, model, solver, num_runs, main_clock_start, 
    simulator_type, belief_type, time_limit,solver_type);
  simulator->world_seed(world_seed);

  int start_run = 0;
//  getInstance(options, simulator, simulator_type, num_runs, start_run);

  /* =========================
   * Display parameters
   * =========================*/
  displayParameters(options, model);

  /* =========================
   * run simulator
   * =========================*/
  runSimulator(model, simulator, options, num_runs, search_solver, solver, simulator_type, 
    main_clock_start, start_run);

  simulator->End();

  printResult(num_runs, simulator, main_clock_start);
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
