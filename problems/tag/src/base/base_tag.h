#ifndef BASETAG_H
#define BASETAG_H

#include "core/pomdp.h"
#include "core/mdp.h"
#include "util/coord.h"
#include "util/floor.h"

/* ==============================================================================
 * TagState class
 * ==============================================================================*/

class TagState: public State {
public:
	TagState();
	TagState(int _state_id);

	string text() const;
};

/* ==============================================================================
 * BaseTag class
 * ==============================================================================*/

class BaseTag: public MDP,
	public BeliefMDP,
	public StateIndexer,
	public StatePolicy,
	public MMAPInferencer {
	friend class TagState;
	friend class TagSHRPolicy;
	friend class TagSPParticleUpperBound;
	friend class TagManhattanUpperBound;
	friend class TagPOMCPPrior;
	friend class TagHistoryModePolicy;

protected:
	static double TAG_REWARD;

	Floor floor_;

	vector<TagState*> states_;
	vector<int> rob_; // rob_[s]: robot cell index for state s
	vector<int> opp_; // opp_[s]: opponent cell index for state s

	vector<vector<vector<State> > > transition_probabilities_; //state, action, [state, weight]
	OBS_TYPE same_loc_obs_;


	mutable MemoryPool<TagState> memory_pool_;

protected:
	map<int, double> OppTransitionDistribution(int state) const;

	void ReadConfig(istream& is);
	void Init(istream& is);
	Coord MostLikelyOpponentPosition(const vector<State*>& particles) const;
	Coord MostLikelyRobPosition(const vector<State*>& particles) const;
	const TagState& MostLikelyState(const vector<State*>& particles) const;
	const State* GetMMAP(const vector<State*>& particles) const;
	void PrintTransitions() const;

protected:
	string RandomMap(int height, int width, int obstacles);
	int NextRobPosition(int rob, int a) const;

	mutable vector<int> default_action_;

public:
  bool robot_pos_unknown_;
	static BaseTag* current_;

	BaseTag();
	BaseTag(string params_file);
	virtual ~BaseTag();

	bool Step(State& state, double random_num, int action,
		double& reward) const;
	virtual bool Step(State& state, double random_num, int action,
		double& reward, OBS_TYPE& obs) const = 0;
	inline int NumActions() const {
		return 5;
	}
	inline int TagAction() const {
		return 4;
	}
	int NumStates() const;
	inline int GetIndex(const State* state) const {
		return state->state_id;
	}
	inline Coord GetRobPos(const State* state) const {
		return floor_.GetCell(rob_[state->state_id]);
	}
	inline int StateIndexToOppIndex(int index) const {
		return index % floor_.NumCells();
	}
	inline int StateIndexToRobIndex(int index) const {
		return index / floor_.NumCells();
	}
	inline int RobOppIndicesToStateIndex(int rob, int opp) const {
		return rob * floor_.NumCells() + opp;
	}
	inline const State* GetState(int index) const {
		return states_[index];
	}

	virtual double ObsProb(OBS_TYPE obs, const State& state, int action) const = 0;
	const vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int a) const;

	State* CreateStartState(string type = "DEFAULT") const;
	virtual Belief* InitialBelief(const State* start, string type = "DEFAULT") const = 0;

	inline double GetMaxReward() const {
		return TAG_REWARD;
	}
	ParticleUpperBound* CreateParticleUpperBound(string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;
	BeliefUpperBound* CreateBeliefUpperBound(string name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, -1);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;
	BeliefLowerBound* CreateBeliefLowerBound(string name = "DEFAULT") const;

	POMCPPrior* CreatePOMCPPrior(string name = "DEFAULT") const;

	void PrintState(const State& state, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE obs, ostream& out = cout) const = 0;
	void PrintAction(int action, ostream& out = cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	const Floor& floor() const;

	int MostLikelyAction(const vector<State*>& particles) const;

	void ComputeDefaultActions(string type) const;
	int GetAction(const State& tagstate) const;

	Belief* Tau(const Belief* belief, int action, OBS_TYPE obs) const;
	void Observe(const Belief* belief, int action, map<OBS_TYPE, double>& obss) const = 0;
	double StepReward(const Belief* belief, int action) const;
};

/* ==============================================================================
 * TagBelief class
 * ==============================================================================*/

class TagBelief: public ParticleBelief {
private:
	const BaseTag* tag_model_;
public:
	TagBelief(vector<State*> particles, const BaseTag* model, Belief* prior =
		NULL);
	void Update(int action, OBS_TYPE obs);
};

#endif
