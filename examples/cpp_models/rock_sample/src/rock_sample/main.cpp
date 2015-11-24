#include <despot/simple_tui.h>
#include "rock_sample.h"

using namespace std;
using namespace despot;

class TUI: public SimpleTUI {
public:
  TUI() {
  }

  DSPOMDP* InitializeModel(option::Option* options) {
    DSPOMDP* model = NULL;
		if (options[E_PARAMS_FILE]) {
			model = new RockSample(options[E_PARAMS_FILE].arg);
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
			model = new RockSample(size, number);
		}
    return model;
  }

  void InitializeDefaultParameters() {
	}
};

int main(int argc, char* argv[]) {
  return TUI().run(argc, argv);
}
