#ifndef SIMPLEROCKSAMPLE_H
#define SIMPLEROCKSAMPLE_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>

namespace despot {

/* =============================================================================
 * SimpleState class
 * =============================================================================*/

class SimpleState: public State {
public:
	int rover_position; // positions are numbered 0, 1, 2 from left to right
	int rock_status; // 1 is good, and 0 is bad

	SimpleState();
	SimpleState(int _rover_position, int _rock_status) : 
        rover_position(_rover_position),
        rock_status(_rock_status) {
    }
	~SimpleState();

	std::string text() const;
};

/* =============================================================================
 * SimpleRockSample class
 * =============================================================================*/

class SimpleRockSample: public DSPOMDP {
protected:
	mutable MemoryPool<SimpleState> memory_pool_;

	std::vector<SimpleState*> states_;

	mutable std::vector<ValuedAction> mdp_policy_;

public:
	enum { // action
		A_SAMPLE = 0, A_EAST = 1, A_WEST = 2, A_CHECK = 3
	};
	enum { // observation
		O_BAD = 0, O_GOOD = 1
	};
	enum { // rock status
		R_BAD = 0, R_GOOD = 1
	};
	enum { // rover position
		LEFT = 0, MIDDLE = 1, RIGHT = 2
	};

public:
	SimpleRockSample();

	/* Returns total number of actions.*/
	int NumActions() const;

	/* Deterministic simulative model.*/
	bool Step(State& state, double rand_num, int action, double& reward,
		OBS_TYPE& obs) const;

	/* Functions related to beliefs and starting states.*/
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;
	State* CreateStartState(std::string type = "DEFAULT") const;
	Belief* InitialBelief(const State* start, std::string type = "DEFAULT") const;

	/* Bound-related functions.*/
	double GetMaxReward() const;
	ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;
	ValuedAction GetMinRewardAction() const;
	ScenarioLowerBound* CreateScenarioLowerBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	/* Memory management.*/
	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	/* Display.*/
	void PrintState(const State& state, std::ostream& out = std::cout) const;
	void PrintBelief(const Belief& belief, std::ostream& out = std::cout) const;
	void PrintObs(const State& state, OBS_TYPE observation,
		std::ostream& out = std::cout) const;
	void PrintAction(int action, std::ostream& out = std::cout) const;
};

} // namespace despot

#endif
