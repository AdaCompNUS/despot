#include <despot/simple_tui.h>
#include "tag.h"

using namespace despot;

class TUI: public SimpleTUI {
public:
  TUI() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = !options[E_PARAMS_FILE] ?
      new Tag() : new Tag(options[E_PARAMS_FILE].arg);
    return model;
  }

  void InitializeDefaultParameters() {
    Globals::config.pruning_constant = 0.01;
  }
};

int main(int argc, char* argv[]) {
  return TUI().run(argc, argv);
}
