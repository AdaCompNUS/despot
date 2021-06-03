/*
 * POMDPWorld.h
 *
 *  Created on: 20 Sep 2017
 *      Author: panpan
 */

#ifndef POMDP_WORLD_H_
#define POMDP_WORLD_H_

#include <despot/core/globals.h>
#include <despot/interface/pomdp.h>
#include <despot/interface/world.h>
#include <despot/util/random.h>

using namespace std;

namespace despot {

class POMDPWorld: public World {
protected:
	//POMDP model shared with despot solver
	DSPOMDP* model_;
	//Random number generator.
	Random random_;
public:
	double step_reward_;

public:
	POMDPWorld(DSPOMDP* model, unsigned seed);
	virtual ~POMDPWorld();
	virtual inline void world_seed(unsigned seed) {
		random_ = Random(seed);
	}

public:
	//establish connection to simulator or system
	bool Connect();
	//Initialize or reset the simulation environment
	State* Initialize();
	//To help construct initial belief to print debug informations in Logger
	State* GetCurrentState() const;
	//Print a world state. Used to print debug information in Logger.
	void PrintState(const State& s, ostream& out) const;
	//send action, receive reward, obs, and terminal
	bool ExecuteAction(ACT_TYPE action, OBS_TYPE& obs);
};

} /* namespace despot */

#endif /* POMDP_WORLD_H_ */
