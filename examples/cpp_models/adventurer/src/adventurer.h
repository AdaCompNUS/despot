#ifndef ADVENTURER_H
#define ADVENTURER_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include <despot/util/coord.h>
#include <despot/solver/pomcp.h>

namespace despot {

/* ==============================================================================
 * AdventurerState class
 * ==============================================================================*/

class AdventurerState: public State {
public:
	AdventurerState();
	AdventurerState(int _state_id);

  std::string text() const;
};

/* ==============================================================================
 * Adventurer class
 * ==============================================================================*/

class Adventurer: public BeliefMDP,
	public MDP,
	public StateIndexer,
	public StatePolicy {
	friend class AdventurerSmartPolicy;
	friend class AdventurerPOMCPPrior;
	friend class AdventurerState;

protected:
	int size_;
	int num_goals_;
  std::vector<double> goal_prob_;
	std::vector<double> goal_reward_;
	double max_goal_reward_;
  std::vector<double> trap_prob_;
	double obs_noise_;
  std::vector<AdventurerState*> states_;

	std::vector<std::vector<std::vector<State> > > transition_probabilities_; //state, action, [state, weight]

	mutable MemoryPool<AdventurerState> memory_pool_;

	std::vector<int> default_action_;

protected:
	void Init(std::istream& is);

	enum {
		A_STAY,
		A_LEFT,
		A_RIGHT
	};

public:
	static Adventurer* current_;

	Adventurer(int num_goals);
	Adventurer(std::string params_file);

	virtual bool Step(State& state, double random_num, int action,
		double& reward, OBS_TYPE& obs) const;
	inline int NumActions() const {
		return 3;
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
		return max_goal_reward_;
	}
	ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, 0);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	void PrintState(const State& state, std::ostream& out = std::cout) const;
	void PrintBelief(const Belief& belief, std::ostream& out = std::cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE obs, std::ostream& out = std::cout) const;
	void PrintAction(int action, std::ostream& out = std::cout) const;

	void PrintTransitions() const;
	void PrintMDPPolicy() const;
	void PrintPOMDPX() const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	void ComputeDefaultActions(std::string type);
	int GetAction(const State& navistate) const;

	Belief* Tau(const Belief* belief, int action, OBS_TYPE obs) const;
	void Observe(const Belief* belief, int action, std::map<OBS_TYPE, double>& obss) const;
	double StepReward(const Belief* belief, int action) const;

	POMCPPrior* CreatePOMCPPrior(std::string name = "DEFAULT") const;
};

} // namespace despot

#endif
