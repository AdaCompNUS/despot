#ifndef REG_DEMO_H
#define REG_DEMO_H

#include "core/pomdp.h"
#include "core/mdp.h"
#include "util/coord.h"

/* ==============================================================================
 * RegDemoState class
 * ==============================================================================*/

class RegDemoState: public State {
public:
	RegDemoState();
	RegDemoState(int _state_id);

	string text() const;
};

/* ==============================================================================
 * RegDemo class
 * ==============================================================================*/

class RegDemo: public DSPOMDP,
	public MDP,
	public StateIndexer,
	public StatePolicy {
protected:
	int size_;
	double goal_reward_;
	vector<double> trap_prob_;
	vector<OBS_TYPE> obs_;
	vector<RegDemoState*> states_;

	vector<vector<vector<State> > > transition_probabilities_; //state, action, [state, weight]

	mutable MemoryPool<RegDemoState> memory_pool_;

	mutable vector<int> default_action_;

protected:
	void Init(istream& is);

	enum {
		A_STAY,
		A_LEFT,
		A_RIGHT
	};

public:
	RegDemo();
	RegDemo(string params_file);

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
	const vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int a) const;

	State* CreateStartState(string type) const;
	virtual Belief* InitialBelief(const State* start, string type = "DEFAULT") const;

	inline double GetMaxReward() const {
		return goal_reward_;
	}
	ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, 0);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	void PrintState(const State& state, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const;
	void PrintAction(int action, ostream& out = cout) const;

	void PrintTransitions() const;
	void PrintMDPPolicy() const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	void ComputeDefaultActions(string type) const;
	int GetAction(const State& navistate) const;
};

#endif
