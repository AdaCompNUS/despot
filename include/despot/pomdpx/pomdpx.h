#ifndef POMDPX_H
#define POMDPX_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include <despot/pomdpx/parser/parser.h>

namespace despot {

/* ==============================================================================
 * POMDPXState class
 * ==============================================================================*/

class POMDPXState: public State {
public:
  std::vector<int> vec_id;

	POMDPXState();
	~POMDPXState();

	POMDPXState(std::vector<int> state);

	std::string text() const;
};

/* ==============================================================================
 * POMDPX class
 * ==============================================================================*/

class POMDPX: public MDP,
	public DSPOMDP,
	public StateIndexer,
	public StatePolicy {
	friend class POMDPXBeliefRevitalise;
	friend class POMDPXBeliefRandomConsistent;
	friend class POMDPXBeliefMix;
	friend class POMDPXGreedyActionPolicy;

private:
	Parser* parser_;
	bool is_small_;

	ValuedAction min_reward_action_;
	ValuedAction max_reward_action_;

	std::vector<POMDPXState*> states_;
	std::vector<std::vector<std::vector<State> > > transition_probabilities_;
	mutable std::vector<std::vector<double> > rewards_;

	mutable MemoryPool<POMDPXState> memory_pool_;

	void InitStates();
	void InitTransitions();
	void PrintTransitions();
	void InitRewards();

	mutable std::vector<int> default_action_;
	void ComputeDefaultActions(std::string type) const;
	void PrintDefaultActions();

	void PrintModel(std::ostream& out = std::cout) const;

public:
	static POMDPX* current_;
	static int STATE_NUM_THRESHOLD;

	POMDPX();
	POMDPX(std::string file);

	inline Parser* parser() const {
		return parser_;
	}

	bool NoisyStep(State& s, double random_num, int action) const;
	bool Step(State& s, double random_num, int action, double& reward,
		OBS_TYPE& obs) const;
	int NumActions() const;
	int NumStates() const;
	int GetIndex(const State* state) const;
	const State* GetState(int index) const;

	const std::vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int action) const;

	double ObsProb(OBS_TYPE obs, const State& s, int a) const;

	State* CreateStartState(std::string type) const;
  std::vector<State*> ExactInitialParticleSet() const;
  std::vector<State*> ApproxInitialParticleSet() const;
	Belief* InitialBelief(const State* start, std::string type = "DEFAULT") const;

	inline int GetAction(const State& state) const {
		const POMDPXState& pomdpx_state = static_cast<const POMDPXState&>(state);
		return default_action_[parser_->ComputeIndex(pomdpx_state.vec_id)];
	}

	inline double GetMaxReward() const {
		return max_reward_action_.value;
	}
	ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return min_reward_action_;
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

	virtual DSPOMDP* MakeCopy() const;

	void PrintMDPBound(const std::vector<ValuedAction>& policy, const char* fn);

	// For server-client messages in IPPC competition
	const std::string& GetActionName();
	const std::string& GetEnumedAction(int action);
	OBS_TYPE GetPOMDPXObservation(std::map<std::string, std::string>& observe);
};

} // namespace despot

#endif
