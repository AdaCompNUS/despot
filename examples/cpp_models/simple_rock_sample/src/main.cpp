#include <despot/evaluator.h>

#include "simple_rock_sample.h"

using namespace despot;

class MyEvaluator: public Evaluator {
public:
  MyEvaluator() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = new SimpleRockSample();
    return model;
  }

  World* InitializeWorld(std::string& world_type, DSPOMDP* model, option::Option* options)
  {
      return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
  }
};

int main(int argc, char* argv[]) {
  return MyEvaluator().runEvaluation(argc, argv);
}
