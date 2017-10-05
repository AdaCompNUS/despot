#include <despot/initializer.h>

#include "simple_rock_sample.h"

using namespace despot;

class MyInitializer: public Initializer {
public:
  MyInitializer() {
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
  return MyInitializer().runEvaluation(argc, argv);
}
