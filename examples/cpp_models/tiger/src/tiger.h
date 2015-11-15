#ifndef TIGER_H
#define TIGER_H

#include <despot/core/pomdp.h>

namespace despot {

/* =============================================================================
 * TigerState class
 * =============================================================================*/

class TigerState: public State {
public:
	int tiger_position;

	TigerState();

	TigerState(int position);

	std::string text() const;
};

/* =============================================================================
 * Tiger class
 * =============================================================================*/

class Tiger: public DSPOMDP {
private:
	mutable MemoryPool<TigerState> memory_pool_;

public:
	static const int LEFT, RIGHT, LISTEN;
	static const double NOISE;

	Tiger();
	Tiger(std::string params_file);

	bool Step(State& s, double random_num, int action, double& reward,
		OBS_TYPE& obs) const;
	int NumStates() const;
	int NumActions() const;
	double ObsProb(OBS_TYPE obs, const State& s, int a) const;

	State* CreateStartState(std::string type) const;
	Belief* InitialBelief(const State* start, std::string type = "DEFAULT") const;

	inline double GetMaxReward() const {
		return 10;
	}

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(LISTEN, -1);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	void PrintState(const State& state, std::ostream& out = std::cout) const;
	void PrintBelief(const Belief& belief, std::ostream& out = std::cout) const;
	void PrintObs(const State& state, OBS_TYPE obs, std::ostream& out = std::cout) const;
	void PrintAction(int action, std::ostream& out = std::cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;
};

} // namespace despot

#endif
