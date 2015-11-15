#include <despot/simple_tui.h>
#include "adventurer.h"

using namespace despot;

class TUI: public SimpleTUI {
public:
	TUI() {
	}

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = NULL;

    int num_goals = options[E_SIZE] ? atoi(options[E_SIZE].arg) : 50;
     model = !options[E_PARAMS_FILE] ?
       new Adventurer(num_goals) : new Adventurer(options[E_PARAMS_FILE].arg);
    return model;
  }

  void InitializeDefaultParameters() {
     Globals::config.search_depth = 5;
     Globals::config.sim_len = 5;
  }
};

int main(int argc, char* argv[]) {
	return TUI().run(argc, argv);
}

