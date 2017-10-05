#include <despot/initializer.h>

#include "bridge.h"

using namespace despot;

class MyInitializer: public Initializer {
public:
	MyInitializer() {
	}

	DSPOMDP* InitializeModel(option::Option* options) {
		DSPOMDP* model = NULL;
		model = new Bridge();
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
