#ifndef SIMPLETUI_H
#define SIMPLETUI_H

#include <typeinfo>
#include <despot/solver/despot.h>
#include <despot/solver/aems.h>
#include <despot/solver/pomcp.h>

#include <despot/util/optionparser.h>
#include <despot/util/seeds.h>

#include <despot/core/pomdp.h>
#include <despot/ippc/client.h>

#include <despot/evaluator.h>

namespace despot {

void disableBufferedIO(void);

enum OptionIndex {
  E_UNKNOWN,
  E_HELP,
  E_PARAMS_FILE,
  E_DEPTH,
  E_DISCOUNT,
  E_SIZE,
  E_NUMBER,
  E_SEED,
  E_TIMEOUT,
  E_NUMPARTICLES,
  E_PRUNE,
  E_GAP,
  E_SIM_LEN,
  E_EVALUATOR,
  E_MAX_POLICY_SIM_LEN,
  E_DEFAULT_ACTION,
  E_RUNS,
  E_BLBTYPE,
  E_LBTYPE,
  E_BUBTYPE,
  E_UBTYPE,
  E_BELIEF,
  E_KNOWLEDGE,
  E_VERBOSITY,
  E_SILENCE,
  E_SOLVER,
  E_TIME_LIMIT,
  E_NOISE,
  E_SEARCH_SOLVER,
  E_PRIOR,
  E_SERVER,
  E_PORT,
  E_LOG,
};

// option::Arg::Required is a misnomer. The program won't complain if these
// are absent, and required flags must be checked manually.
const option::Descriptor usage[] = {
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
  // { E_EVALUATOR, 0, "", "evaluator", option::Arg::Required, "  \t--evaluator
  // <arg>  \tUse IPPC server or a POMDP model as the evaluator." },
  // { E_DEFAULT_ACTION, 0, "", "default-action", option::Arg::Required, "
  // \t--default-action <arg>  \tType of default action to use. (default none)."
  // },
  { E_LBTYPE, 0, "l", "lbtype", option::Arg::Required,
    "-l <arg>  \t--lbtype <arg>  \tLower bound strategy." },
  { E_BLBTYPE, 0, "", "blbtype", option::Arg::Required,
    "  \t--blbtype <arg>  \tBase lower bound." },
  { E_UBTYPE, 0, "u", "ubtype", option::Arg::Required,
    "-u <arg>  \t--ubtype <arg>  \tUpper bound strategy." },
  { E_BUBTYPE, 0, "", "bubtype", option::Arg::Required,
    "  \t--bubtype <arg>  \tBase upper bound." },

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
  // { E_TIME_LIMIT, 0, "", "time-limit", option::Arg::Required, "
  // \t--time-limit <arg>  \tTotal amount of time allowed for the program." },
  // { E_SEARCH_SOLVER, 0, "", "search-solver", option::Arg::None, "
  // \t--search-solver\tUse first few runs to select DESPOT or POMCP as the
  // solver for remaining runs." },
  { E_PRIOR, 0, "", "prior", option::Arg::Required, 
    "  \t--prior <arg>  \tPOMCP prior." },
  // { E_SERVER, 0, "", "server", option::Arg::Required, "  \t--server <arg>
  // \tServer address." },
  // { E_PORT, 0, "", "port", option::Arg::Required, "  \t--port <arg>  \tPort
  // number." },
  // { E_LOG, 0, "", "log", option::Arg::Required, "  \t--log <arg>  \tIPPC log
  // file." },
  { 0, 0, 0, 0, 0, 0 }
};

class SimpleTUI {
public:
  SimpleTUI();
  virtual ~SimpleTUI();

  virtual DSPOMDP* InitializeModel(option::Option* options) = 0;
  virtual void InitializeDefaultParameters() = 0;

  Solver* InitializeSolver(DSPOMDP* model, std::string solver_type,
                           option::Option* options);

  int run(int argc, char* argv[]);

  void OptionParse(option::Option* options, int& num_runs,
                   std::string& simulator_type, std::string& belief_type, int& time_limit,
                   std::string& solver_type, bool& search_solver);

  void InitializeEvaluator(Evaluator*& simulator, option::Option* options,
                           DSPOMDP* model, Solver* solver, int num_runs,
                           clock_t main_clock_start, std::string simulator_type,
                           std::string belief_type, int time_limit,
                           std::string solver_type);

  void DisplayParameters(option::Option* options, DSPOMDP* model);

  void RunEvaluator(DSPOMDP* model, Evaluator* simulator,
                    option::Option* options, int num_runs, bool search_solver,
                    Solver*& solver, std::string simulator_type,
                    clock_t main_clock_start, int start_run);

  void PrintResult(int num_runs, Evaluator* simulator,
                   clock_t main_clock_start);
};

} // namespace despot

#endif
