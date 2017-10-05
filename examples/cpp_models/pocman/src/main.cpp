#include <despot/initializer.h>

#include "pocman.h"

using namespace despot;

class MyInitializer: public Initializer {
public:
  MyInitializer() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = new FullPocman();
    return model;
  }

  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
      return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
     Globals::config.num_scenarios = 100;
  }
};

int main(int argc, char* argv[]) {
  return MyInitializer().runEvaluation(argc, argv);
}
