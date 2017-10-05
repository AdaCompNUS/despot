#include <despot/initializer.h>
#include "noisy_laser_tag.h"

using namespace despot;

class MyInitializer: public Initializer {
public:
  MyInitializer() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = !options[E_PARAMS_FILE] ?
      new NoisyLaserTag() : new NoisyLaserTag(options[E_PARAMS_FILE].arg);
    return model;
  }

  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
      return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
    Globals::config.pruning_constant = 0.01;
  }
};

int main(int argc, char* argv[]) {
  return MyInitializer().runEvaluation(argc, argv);
}
