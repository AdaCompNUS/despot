#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "navigation.h"
#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include <despot/util/coord.h>

namespace despot {

/* ==============================================================================
 * NavigationState class
 * ==============================================================================*/

class NavigationState: public State {
public:
	NavigationState();
	NavigationState(int _state_id);

  std::string text() const;
};

/* ==============================================================================
 * Navigation class
 * ==============================================================================*/

class Navigation: public DSPOMDP,
	public MDP,
	public StateIndexer,
	public StatePolicy {
protected:
	static int flag_size_; // first 3 bits in state index: first_step_done, second_step_done, min-x
	static int flag_bits_;

	int xsize_, ysize_;
	int goal_pos_, trap_pos_;

	std::vector<double> trap_prob_; // trap_prob_[y * xsize_ + x]
	std::vector<OBS_TYPE> obs_; // obs_[y * xsize_ + x]
	std::vector<NavigationState*> states_;

	std::vector<std::vector<std::vector<State> > > transition_probabilities_; //state, action, [state, weight]

	mutable MemoryPool<NavigationState> memory_pool_;

	std::vector<int> default_action_;

protected:
	void Init(std::istream& is);
	int NextPosition(int pos, int action) const;

public:
	Navigation();
	Navigation(std::string params_file);

	virtual bool Step(State& state, double random_num, int action,
		double& reward, OBS_TYPE& obs) const;
	inline int NumActions() const {
		return 5;
	}
	int NumStates() const;
	inline int GetIndex(const State* state) const {
		return state->state_id;
	}
	inline const State* GetState(int index) const {
		return states_[index];
	}

	virtual double ObsProb(OBS_TYPE obs, const State& state, int action) const;
	const std::vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int a) const;

	State* CreateStartState(std::string type) const;
	virtual Belief* InitialBelief(const State* start, std::string type = "DEFAULT") const;

	inline double GetMaxReward() const {
		return 0;
	}
	ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, -1);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	void PrintState(const State& state, std::ostream& out = std::cout) const;
	void PrintBelief(const Belief& belief, std::ostream& out = std::cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE obs, std::ostream& out = std::cout) const;
	void PrintAction(int action, std::ostream& out = std::cout) const;

	void PrintTransitions() const;
	void PrintMDPPolicy() const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	void ComputeDefaultActions(std::string type);
	int GetAction(const State& navistate) const;
};

} // namespace despot

#endif
