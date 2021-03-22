#ifndef SIMPLETUI_H
#define SIMPLETUI_H

#include <typeinfo>
#include <despot/solver/despot.h>
#include <despot/solver/aems.h>
#include <despot/solver/pomcp.h>

#include <despot/util/optionparser.h>
#include <despot/util/seeds.h>

#include <despot/interface/pomdp.h>
#include <despot/interface/world.h>
#include <despot/logger.h>

using namespace std;

namespace despot {

void DisableBufferedIO(void);

enum OptionIndex {
	E_UNKNOWN,
	E_HELP,
	E_PARAMS_FILE,
	E_WORLD_FILE,
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
	E_WORLD,
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

option::Descriptor* BuildUsage(string lower_bounds_str,
		string base_lower_bounds_str, string upper_bounds_str,
		string base_upper_bounds_str);

// option::Arg::Required is a misnomer. The program won't complain if these
// are absent, and required flags must be checked manually.
const option::Descriptor usage[] =
		{ { E_HELP, 0, "", "help", option::Arg::None,
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
				{ E_MAX_POLICY_SIM_LEN, 0, "", "max-policy-simlen",
						option::Arg::Required,
						"  \t--max-policy-simlen <arg>  \tDepth to simulate the default policy "
								"until. (default 90)." },

				{ E_SEED, 0, "r", "seed", option::Arg::Required,
						"-r <arg>  \t--seed <arg>  \tRandom number seed (default is random)." },
				{ E_SIM_LEN, 0, "s", "simlen", option::Arg::Required,
						"-s <arg>  \t--simlen <arg>  \tNumber of steps to simulate. (default 90; 0 "
								"= infinite)." }, { E_RUNS, 0, "", "runs",
						option::Arg::Required,
						"  \t--runs <arg>  \tNumber of runs. (default 1)." }, {
						E_LBTYPE, 0, "l", "lbtype", option::Arg::Required,
						"-l <arg>  \t--lbtype <arg>  \tLower bound strategy." },
				{ E_BLBTYPE, 0, "", "blbtype", option::Arg::Required,
						"  \t--blbtype <arg>  \tBase lower bound." }, {
						E_UBTYPE, 0, "u", "ubtype", option::Arg::Required,
						"-u <arg>  \t--ubtype <arg>  \tUpper bound strategy." },
				{ E_BUBTYPE, 0, "", "bubtype", option::Arg::Required,
						"  \t--bubtype <arg>  \tBase upper bound." },

				{ E_BELIEF, 0, "b", "belief", option::Arg::Required,
						"-b <arg>  \t--belief <arg>  \tBelief update strategy, if applicable." },
				{ E_NOISE, 0, "", "noise", option::Arg::Required,
						"  \t--noise <arg>  \tNoise level for transition in POMDPX belief "
								"update." },

				{ E_VERBOSITY, 0, "v", "verbosity", option::Arg::Required,
						"-v <arg>  \t--verbosity <arg>  \tVerbosity level." }, {
						E_SILENCE, 0, "", "silence", option::Arg::None,
						"  \t--silence  \tReduce default output to minimal." },
				{ E_SOLVER, 0, "", "solver", option::Arg::Required,
						"  \t--solver <arg>  \t" }, { E_PRIOR, 0, "", "prior",
						option::Arg::Required,
						"  \t--prior <arg>  \tPOMCP prior." },
				{ E_PRIOR, 0, "", "world", option::Arg::Required,
						"  \t--world <arg>  \tWorld type (pomdp, simulator, or real)." },
				{ 0, 0, 0, 0, 0, 0 } };

/* =============================================================================
 * Initializer class
 * =============================================================================*/
/**
 * Pipeline control of planning and evaluation processes.
 */
class PlannerBase {
private:
	option::Descriptor* usage;

public:
	PlannerBase(string lower_bounds_str = "TRIVIAL",
			string base_lower_bounds_str = "TRIVIAL", string upper_bounds_str =
					"TRIVIAL", string base_upper_bounds_str = "TRIVIAL");

	virtual ~PlannerBase();

	/**
	 * [Essential]
	 * Create, initialize, and return a DSPOMDP model
	 *
	 * @param options Parsed command line options
	 */
	virtual DSPOMDP* InitializeModel(option::Option* options) = 0;

	/**
	 * [Essential]
	 * Create, initialize, and return the world
	 *
	 * @param world_type Type of the world: pomdp, simulator, or real-world
	 * @param model      The POMDP model
	 * @param options    Parsed command line options
	 */
	virtual World* InitializeWorld(std::string& world_type, DSPOMDP *model,
			option::Option* options)=0;

	/**
	 * [Essential]
	 * Provide default values for global parameters (such as those in Globals::config)
	 */
	virtual void InitializeDefaultParameters() = 0;

	/**
	 * [Essential]
	 * Return the name of the intended solver ("DESPOT", "AEMS2", "POMCP", "DPOMCP", "PLB", "BLB")
	 */
	virtual std::string ChooseSolver()=0;

	/**
	 * Initialize a DSPOMDP model-based world
	 * To be called when using a POMDP-based world
	 */
	World* InitializePOMDPWorld(std::string& world_type, DSPOMDP *model,
			option::Option* options);

	/**
	 * Initialize global parameters according command-line arguments
	 */
	option::Option* InitializeParamers(int argc, char *argv[],
			std::string& solver_type, bool& search_solver, int& num_runs,
			std::string& simulator_type, std::string& belief_type,
			int& time_limit);

	/**
	 * Initialize the solver according to user-specified solver type (defualt "DESPOT")
	 */
	Solver* InitializeSolver(DSPOMDP* model, Belief* belief,
			std::string solver_type, option::Option* options);

	/**
	 * Parse global parameters from command-line arguments
	 */
	void OptionParse(option::Option* options, int& num_runs,
			std::string& simulator_type, std::string& belief_type,
			int& time_limit, std::string& solver_type, bool& search_solver);

	/**
	 * Initialize the statistics logger
	 */
	void InitializeLogger(Logger*& logger, option::Option* options,
			DSPOMDP* model, Belief* belief, Solver* solver, int num_runs,
			clock_t main_clock_start, World* world, std::string world_type,
			int time_limit, std::string solver_type);

	/**
	 * Display gloabl parameters
	 */
	void DisplayParameters(option::Option* options, DSPOMDP* model);

	/**
	 * Print time records and statistics results
	 */
	void PrintResult(int num_runs, Logger* simulator,
			clock_t main_clock_start);
};

} // namespace despot

#endif
