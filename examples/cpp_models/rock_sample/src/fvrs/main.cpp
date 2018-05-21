#include <despot/planner.h>
#include "fvrs.h"

using namespace std;
using namespace despot;

class MyPlanner: public Planner {
public:
  MyPlanner() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = NULL;
    if (options[E_PARAMS_FILE]) {
      model = new FVRS(options[E_PARAMS_FILE].arg);
    } else {
      int size = 7, number = 8;
      if (options[E_SIZE])
        size = atoi(options[E_SIZE].arg);
      else {
        cerr << "Specify map size using --size option" << endl;
        exit(0);
      }
      if (options[E_NUMBER]) {
        number = atoi(options[E_NUMBER].arg);
      } else {
        cerr << "Specify number of rocks using --number option" << endl;
        exit(0);
      }
      model = new FVRS(size, number);
    }
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
  return MyPlanner().RunEvaluation(argc, argv);
}

