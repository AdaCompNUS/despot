#include "simple_tui.h"
#include "reg_demo.h"

class TUI: public SimpleTUI {
public:
  TUI() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = !options[E_PARAMS_FILE] ?
      new RegDemo() : new RegDemo(options[E_PARAMS_FILE].arg);
    return model;
  }

  void InitializeDefaultParameters() {
  }
};

int main(int argc, char* argv[]) {
  return TUI().run(argc, argv);
}


