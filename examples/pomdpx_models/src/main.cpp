#include <despot/planner.h>
#include <despot/pomdpx/pomdpx.h>

using namespace std;
using namespace despot;

class MyPlanner: public Planner {
public:
  MyPlanner() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = NULL;
    if (options[E_PARAMS_FILE]) {
      model = new POMDPX(options[E_PARAMS_FILE].arg);
    } else {
      cerr << "ERROR: Specify a POMDPX model file name using -m!" << endl;
      exit(0);
    }
    return model;
  }
  
  World* InitializeWorld(std::string&  world_type, DSPOMDP* model, option::Option* options)
  {
		if (options[E_WORLD_FILE]) { // ignore the provided model
			DSPOMDP* world = new POMDPX(options[E_WORLD_FILE].arg);
			POMDPX::current_ = (POMDPX*) world;

			return InitializePOMDPWorld(world_type, world, options);
		} else {
			return InitializePOMDPWorld(world_type, model, options);
		}
  }

  void InitializeDefaultParameters() {
  }

  std::string ChooseSolver(){
	  return "DESPOT";
  }
};

int main(int argc, char* argv[]) {
  return MyPlanner().RunEvaluation(argc, argv);
}
