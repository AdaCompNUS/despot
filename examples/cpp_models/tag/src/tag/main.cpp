#include <despot/planner.h>
#include "tag.h"

using namespace despot;

class MyPlanner: public Planner {
public:
  MyPlanner(string lower_bound_str,
			string base_lower_bound_str,
			string upper_bound_str,
			string base_upper_bound_str) : Planner(lower_bound_str,
				base_lower_bound_str,
				upper_bound_str,
				base_lower_bound_str) {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = !options[E_PARAMS_FILE] ?
      new Tag() : new Tag(options[E_PARAMS_FILE].arg);
    return model;
  }

  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
      return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
    Globals::config.pruning_constant = 0.01;
  }

  std::string ChooseSolver(){
	  return "DESPOT";
  }
};

using namespace std;

int main(int argc, char* argv[]) {
	return MyPlanner("w/o base lower bounds: TRIVIAL; with base lower bounds: RANDOM, SHR, MODE-MDP, MODE-SP, MAJORITY-MDP, MAJORITY-SP (default to MODE-MDP)",
			"TRIVIAL (default)",
			"w/o base lower bounds: TRIVIAL, MDP, SP, MANHATTAN; with base lower bounds: LOOKAHEAD -- default to SP",
			"TRIVIAL, MDP, SP, MANHATTAN (default to SP)").RunEvaluation(argc, argv);
}
