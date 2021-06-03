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

using namespace std;

namespace despot {

/* =============================================================================
 * World class
 * =============================================================================*/
/**
 * [Essential]
 * Interface for communication with the real world.
 */
class World {
protected:
	// True state of the (simulated) world
	State* state_;

public:
	World();
	virtual ~World();

public:
	/**
	 * [Essential]
	 * Establish connection to simulator or system
	 */
	virtual bool Connect()=0;

	/**
	 * [Essential]
	 * Initialize or reset the (simulation) environment, return the start state if applicable
	 */
	virtual State* Initialize()=0;

	/**
	 * [Optional]
	 * To help construct initial belief to print debug informations in Logger
	 */
	virtual State* GetCurrentState() const;

	/**
	 * [Optional]
	 * Print a world state. Used to print debug information in Logger.
	 */
	virtual void PrintState(const State& s, ostream& out) const;

	/**
	 * [Essential]
	 * send action, receive reward, obs, and terminal
	 * @param action Action to be executed in the real-world system
	 * @param obs    Observation sent back from the real-world system
	 */
	virtual bool ExecuteAction(ACT_TYPE action, OBS_TYPE& obs) =0;
};

} /* namespace despot */

#endif /* DESPOT_WORLD_H_ */
