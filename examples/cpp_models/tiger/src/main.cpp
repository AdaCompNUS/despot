#include <despot/simple_tui.h>
#include "tiger.h"

using namespace despot;

class TUI: public SimpleTUI {
public:
  TUI() {
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
  return TUI().run(argc, argv);
}
