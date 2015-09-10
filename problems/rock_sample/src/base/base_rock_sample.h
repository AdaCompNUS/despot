#ifndef BASEROCKSAMPLE_H
#define BASEROCKSAMPLE_H

#include "core/pomdp.h"
#include "solver/pomcp.h"
#include "core/mdp.h"
#include "util/coord.h"
#include "util/grid.h"

/* =============================================================================
 * RockSampleState class
 * =============================================================================*/

class RockSampleState: public State {
public:
	RockSampleState();
	RockSampleState(int _state_id);

	string text() const;
};

/* =============================================================================
 * BaseRockSample class
 * =============================================================================*/

class BaseRockSample: public MDP,
	public BeliefMDP,
	public StateIndexer,
	public StatePolicy {
	friend class RockSampleENTScenarioLowerBound;
	friend class RockSampleMMAPStateScenarioLowerBound;
	friend class RockSampleEastScenarioLowerBound;
	friend class RockSampleParticleUpperBound1;
	friend class RockSampleParticleUpperBound2;
	friend class RockSampleMDPParticleUpperBound;
	friend class RockSampleApproxParticleUpperBound;
	friend class RockSampleEastBeliefPolicy;
	friend class RockSampleMDPBeliefUpperBound;
	friend class RockSamplePOMCPPrior;

protected:
	Grid<int> grid_;
	vector<Coord> rock_pos_;
	int size_, num_rocks_;
	Coord start_pos_;
	double half_efficiency_distance_;

	RockSampleState* rock_state_;
	mutable MemoryPool<RockSampleState> memory_pool_;

	vector<RockSampleState*> states_;
protected:
	void InitGeneral();
	void Init_4_4();
	void Init_5_5();
	void Init_5_7();
	void Init_7_8();
	void Init_11_11();
	void InitStates();
	bool GetObservation(double rand_num, const RockSampleState& rockstate,
		int rock) const;

	vector<vector<vector<State> > > transition_probabilities_;
	vector<vector<double> > alpha_vectors_; // For blind policy
	mutable vector<ValuedAction> mdp_policy_;

public:
	enum { // FRAGILE: Don't change!
		E_BAD = 0,
		E_GOOD = 1,
		E_NONE = 2
	};

	enum { // FRAGILE: Don't change!
		E_SAMPLE = 4
	};

public:
	BaseRockSample(string map);
	BaseRockSample(int size, int rocks);

	virtual bool Step(State& state, double rand_num, int action,
		double& reward, OBS_TYPE& obs) const = 0;
	virtual int NumActions() const = 0;
	virtual double ObsProb(OBS_TYPE obs, const State& state, int action) const = 0;

	const vector<State>& TransitionProbability(int s, int a) const;
	int NextState(int s, int a) const;
	double Reward(int s, int a) const;

	State* CreateStartState(string type = "DEFAULT") const;
	vector<State*> InitialParticleSet() const;
	vector<State*> NoisyInitialParticleSet() const;
	Belief* InitialBelief(const State* start, string type = "PARTICLE") const;

	inline double GetMaxReward() const {
		return 10;
	}
	ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;
	BeliefUpperBound* CreateBeliefUpperBound(string name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(E_SAMPLE+1, 0);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;
	BeliefLowerBound* CreateBeliefLowerBound(string name = "DEFAULT") const;

	POMCPPrior* CreatePOMCPPrior(string name = "DEFAULT") const;

	void PrintState(const State& state, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE observation, ostream& out = cout) const = 0;
	void PrintAction(int action, ostream& out = cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	Belief* Tau(const Belief* belief, int action, OBS_TYPE obs) const;
	void Observe(const Belief* belief, int action, map<OBS_TYPE, double>& obss) const;
	double StepReward(const Belief* belief, int action) const;

	int NumStates() const;
	const State* GetState(int index) const;
	int GetIndex(const State* state) const;

	inline int GetAction(const State& tagstate) const {
		return 0;
	}

	int GetRobPosIndex(const State* state) const;
	Coord GetRobPos(const State* state) const;
	bool GetRock(const State* state, int rock) const;
	void SampleRock(State* state, int rock) const;
	int GetX(const State* state) const;
	void IncX(State* state) const;
	void DecX(State* state) const;
	int GetY(const State* state) const;
	void IncY(State* state) const;
	void DecY(State* state) const;

protected:
	void InitializeTransitions();
	Coord IndexToCoord(int pos) const;
	int CoordToIndex(Coord c) const;
	vector<ValuedAction>& ComputeOptimalSamplingPolicy() const;
	RockSampleState* MajorityRockSampleState(const vector<State*>& particles) const;
};

#endif
