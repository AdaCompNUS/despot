#include "navigation.h"
#include <despot/core/builtin_lower_bounds.h>
#include <despot/core/builtin_policy.h>
#include <despot/core/builtin_upper_bounds.h>
#include <despot/core/particle_belief.h>
#include <despot/evaluator.h>

using namespace despot;

class MyEvaluator: public Evaluator {
public:
  MyEvaluator() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = !options[E_PARAMS_FILE] ?
      new Navigation() : new Navigation(options[E_PARAMS_FILE].arg);
    return model;
  }

  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
      return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
  }

  std::string ChooseSolver(){
	  return "DESPOT";
  }
};

int main(int argc, char* argv[]) {
  return MyEvaluator().runEvaluation(argc, argv);
}


