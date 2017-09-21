/*
 * World.h
 *
 *  Created on: 20 Sep 2017
 *      Author: panpan
 */

#ifndef DESPOT_WORLD_H_
#define DESPOT_WORLD_H_
#include <despot/core/globals.h>
#include <despot/interface/pomdp.h>

namespace despot {

class World {
protected:
// True state of the (simulated) world
	State* state_;

public:
	World();
	virtual ~World();

public:
	//establish connection to simulator or system
	virtual bool Connect()=0;
	//Initialize or reset the (simulation) environment, return the start state if applicable
	virtual State* Initialize()=0;
	//To help construct initial belief to print debug informations in Evaluator
	virtual State* GetTrueState() const = 0;
	//send action, receive reward, obs, and terminal
	virtual bool ExecuteAction(int action, OBS_TYPE& obs) =0;
};

} /* namespace despot */

#endif /* DESPOT_WORLD_H_ */
