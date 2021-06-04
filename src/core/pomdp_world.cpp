/*
 * POMDPWorld.cpp
 *
 *  Created on: 20 Sep 2017
 *      Author: panpan
 */

#include <despot/core/pomdp_world.h>

namespace despot {

POMDPWorld::POMDPWorld(DSPOMDP* model, unsigned seed) :
		model_(model) {
	random_ = Random(seed);
}

POMDPWorld::~POMDPWorld() {
}

bool POMDPWorld::Connect() {
	return true;
}

State* POMDPWorld::Initialize() {
	state_ = model_->CreateStartState();
	return state_;
}

despot::State* POMDPWorld::GetCurrentState() const {
	return state_;
}

void POMDPWorld::PrintState(const State& s, ostream& out) const {
	model_->PrintState(s, out);
}

bool POMDPWorld::ExecuteAction(ACT_TYPE action, OBS_TYPE& obs) {
	if (action >= model_->NumActions()) {
		cout << "WARNING: Planned action " << action << 
			" set to maximum allowed value " << (model_->NumActions() - 1) << endl;
		action = model_->NumActions() - 1;
	}
	bool terminal = model_->Step(*state_, random_.NextDouble(), action,
			step_reward_, obs);
	return terminal;
}

} /* namespace despot */
