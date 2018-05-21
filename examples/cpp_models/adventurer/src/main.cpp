#include <despot/planner.h>

#include "adventurer.h"

using namespace despot;

class MyPlanner: public Planner {
public:
	MyPlanner() {
	}

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = NULL;

    int num_goals = options[E_SIZE] ? atoi(options[E_SIZE].arg) : 50;
     model = !options[E_PARAMS_FILE] ?
       new Adventurer(num_goals) : new Adventurer(options[E_PARAMS_FILE].arg);
    return model;
  }

  World* InitializeWorld(std::string& world_type, DSPOMDP* model, option::Option* options)
  {
	 return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
     Globals::config.search_depth = 5;
     Globals::config.sim_len = 5;
  }

  std::string ChooseSolver(){
	  return "DESPOT";
  }
};

int main(int argc, char* argv[]) {
	return MyPlanner().RunEvaluation(argc, argv);
}

