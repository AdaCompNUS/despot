/*
 * POMDPWorld.h
 *
 *  Created on: 20 Sep 2017
 *      Author: panpan
 */

#ifndef INCLUDE_DESPOT_CORE_POMDP_WORLD_H_
#define INCLUDE_DESPOT_CORE_POMDP_WORLD_H_

#include <despot/core/globals.h>
#include <despot/core/pomdp.h>
#include <despot/core/world.h>
#include <despot/util/random.h>

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
	//To help construct initial belief to print debug informations in Evaluator
	State* GetTrueState() const;
	//send action, receive reward, obs, and terminal
	bool ExecuteAction(int action, OBS_TYPE& obs);
};

} /* namespace despot */

#endif /* INCLUDE_DESPOT_CORE_POMDP_WORLD_H_ */
