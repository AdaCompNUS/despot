#include <despot/initializer.h>

#include "tiger.h"

using namespace despot;

class MyInitializer: public Initializer {
public:
  MyInitializer() {
  }
 
  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = new Tiger();
    return model;
  }
  
  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
	return InitializePOMDPWorld(world_type, model, options);
  }

  void InitializeDefaultParameters() {
  }
};

int main(int argc, char* argv[]) {
  return MyInitializer().runEvaluation(argc, argv);
}
