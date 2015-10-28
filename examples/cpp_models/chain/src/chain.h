#ifndef CHAIN_H
#define CHAIN_H

#include "core/pomdp.h"
#include "util/dirichlet.h"

/* =============================================================================
 * ChainState class
 * =============================================================================*/

class ChainState: public State {
private:
	vector<vector<vector<double> > > mdp_transitions_; //mdp_transitions_[s1][a][s2] = P(s2|s1,a)

public:
	int mdp_state;
	vector<ValuedAction> policy;// optimal policy

	ChainState();

	void Init(int num_mdp_states, int num_mdp_actions);
	bool IsValid() const;

	void SetTransition(int state, int action, vector<double> row);
	void SetTransition(int state1, int action, int state2, double value) {
		mdp_transitions_[state1][action][state2] = value;
	}
	vector<double> GetTransition(int state, int action) {
		return mdp_transitions_[state][action];
	}
	inline double GetTransition(int state1, int action, int state2) {
		return mdp_transitions_[state1][action][state2];
	}

	void ComputeOptimalPolicy();

	string text() const;
};

/* =============================================================================
 * SemiChainBelief class
 * =============================================================================*/

class SemiChainBelief: public Belief {
private:
	enum {
		SUCCESS, SLIP
	};
	//alpha_[a]: Dirichlet hyper-parameter for P(R|a), R = success, slip
	vector<vector<double> > alpha_;
	int cur_state_;

public:
	SemiChainBelief(const DSPOMDP* model, int num_mdp_states,
		int num_mdp_actions);

	void Update(int action, OBS_TYPE obs);

	vector<State*> Sample(int num_particles) const;

	Belief* MakeCopy() const;

	string text() const;
};

/* =============================================================================
 * FullChainBelief class
 * =============================================================================*/

class FullChainBelief: public Belief {
private:
	//alpha_[s][a]: Dirichlet hyper-parameter for P(s'|s,a)
	vector<vector<vector<double> > > alpha_;
	int cur_state_;

public:
	FullChainBelief(const DSPOMDP* model, int num_mdp_states, int num_mdp_actions, double alpha = 1.0);

	void Update(int action, OBS_TYPE obs);

	vector<State*> Sample(int num_particles) const;

	Belief* MakeCopy() const;

	string text() const;
};

/* =============================================================================
 * Chain class
 * =============================================================================*/

class Chain: public DSPOMDP {
private:
	mutable MemoryPool<ChainState> memory_pool_;
	double alpha_;

public:
	enum {
		ACTION_A, ACTION_B
	};

	const static int NUM_MDP_STATES = 5;
	const static int INITIAL_MDP_STATE = 0;

	Chain();
	Chain(string fn);

	bool Step(State& s, double random_num, int action, double& reward,
		OBS_TYPE& obs) const;
	double Reward(int s1, int action, int s2) const;
	int NumActions() const;
	double ObsProb(OBS_TYPE obs, const State& state, int action) const;

	State* DefaultStartState() const;
	State* CreateStartState(string type = "DEFAULT") const;
	Belief* InitialBelief(const State* start, string type) const;

	inline double GetMaxReward() const {
		return 10;
	}
	ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(ACTION_B, 0);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	void ComputeOptimalValue(ChainState& state) const;

	void PrintState(const State& s, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;
	void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const;
	void PrintAction(int action, ostream& out = cout) const;

	virtual State* Allocate(int state_id, double weight) const;
	virtual State* Copy(const State* particle) const;
	virtual void Free(State* particle) const;
	int NumActiveParticles() const;
};

#endif
