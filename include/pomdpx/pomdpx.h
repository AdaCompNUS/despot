#ifndef POMDPX_H
#define POMDPX_H

#include "core/pomdp.h"
#include "core/mdp.h"
#include "pomdpx/parser/parser.h"

using namespace std;

/* ==============================================================================
 * POMDPXState class
 * ==============================================================================*/

class POMDPXState: public State {
public:
	vector<int> vec_id;

	POMDPXState();
	~POMDPXState();

	POMDPXState(vector<int> state);

	string text() const;
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

	vector<POMDPXState*> states_;
	vector<vector<vector<State> > > transition_probabilities_;
	mutable vector<vector<double> > rewards_;

	mutable MemoryPool<POMDPXState> memory_pool_;

	void InitStates();
	void InitTransitions();
	void PrintTransitions();
	void InitRewards();

	mutable vector<int> default_action_;
	void ComputeDefaultActions(string type) const;
	void PrintDefaultActions();

	void PrintModel(ostream& out = cout) const;

public:
	static POMDPX* current_;
	static int STATE_NUM_THRESHOLD;

	POMDPX();
	POMDPX(string file);

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

	const vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int action) const;

	double ObsProb(OBS_TYPE obs, const State& s, int a) const;

	State* CreateStartState(string type) const;
	vector<State*> ExactInitialParticleSet() const;
	vector<State*> ApproxInitialParticleSet() const;
	Belief* InitialBelief(const State* start, string type = "DEFAULT") const;

	inline int GetAction(const State& state) const {
		const POMDPXState& pomdpx_state = static_cast<const POMDPXState&>(state);
		return default_action_[parser_->ComputeIndex(pomdpx_state.vec_id)];
	}

	inline double GetMaxReward() const {
		return max_reward_action_.value;
	}
	ParticleUpperBound* CreateParticleUpperBound(string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return min_reward_action_;
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	void PrintState(const State& state, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;
	void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const;
	void PrintAction(int action, ostream& out = cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	virtual DSPOMDP* MakeCopy() const;

	void PrintMDPBound(const vector<ValuedAction>& policy, const char* fn);

	// For server-client messages in IPPC competition
	const string& GetActionName();
	const string& GetEnumedAction(int action);
	OBS_TYPE GetPOMDPXObservation(map<string, string>& observe);
};

#endif
