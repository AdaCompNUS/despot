#ifndef FASTTAG_H
#define FASTTAG_H

// #include "tag.h"
#include "util/floor.h"
#include "pomdp.h"
#include "mdp.h"
#include "util/coord.h"

/*---------------------------------------------------------------------------*/

class FastTagState : public State {
public:
	FastTagState();
	FastTagState(int _state_id);

	string text() const;
};

/*---------------------------------------------------------------------------*/

class FastTag : public MDP, public BeliefMDP, public StateIndexer, public StatePolicy, public MMAPInferencer {
friend class FastTagSHRPolicy;
friend class FastTagSPParticleUpperBound;
friend class FastTagManhattanParticleUpperBound;
friend class FastTagManhattanBeliefUpperBound;

protected:
  static double TAG_REWARD;

	Floor floor_;
	FastTagState* start_state_;

	vector<FastTagState*> states_;
	vector<int> rob_;
	vector<int> opp_;
	vector<uint64_t> obs_;

	vector<vector<vector<State> > > transition_probabilities_; //state, action, [state, weight]
	uint64_t same_loc_obs_;

	mutable MemoryPool<FastTagState> memory_pool_;

protected:
	map<int, double> OppTransitionDistribution(int state) const;

	void ReadConfig(istream& is);
	void Init(istream& is);
	Coord MostLikelyOpponentPosition(const vector<State*>& particles) const;
	Coord MostLikelyRobPosition(const vector<State*>& particles) const;
	const FastTagState& MostLikelyState(const vector<State*>& particles) const;
	const State* GetMMAP(const vector<State*>& particles) const;
	void PrintTransitions() const;

	static vector<double> state_weights_;

protected:
	int NextRobPosition(int rob, int a) const;

	mutable vector<int> default_action_;

public:
	static FastTag* current_;

	FastTag();
	FastTag(string params_file);

	virtual bool Step(State& state, double random_num, int action,
			double& reward, uint64_t& obs) const;
  inline int NumActions() const { return 5; }
  inline int TagAction() const { return 4; }
  int NumStates() const;
	inline int GetIndex(const State* state) const {
		return state->state_id;
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
	inline const State* GetState(int index) const { return states_[index]; }

  virtual double ObsProb(uint64_t obs, const State& state, int action) const;
	const vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int a) const;

  State* CreateStartState(string type = "DEFAULT") const;
  State* RandomStartState() const;
  virtual Belief* ExactPrior(const State* start) const;
  virtual Belief* InitialBelief(const State* start, string type = "DEFAULT") const;

	inline double GetMaxReward() const {
		return TAG_REWARD;
	}
	ParticleUpperBound* CreateParticleUpperBound(string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(string name,
		string particle_bound_name = "DEFAULT") const;
	BeliefUpperBound* CreateBeliefUpperBound(string name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, -1);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(string name, 
		string particle_bound_name = "DEFAULT") const;
	BeliefLowerBound* CreateBeliefLowerBound(string name = "DEFAULT") const;

	void PrintAction(int a, ostream& out) const;
  void PrintState(const State& state, ostream& out = cout) const;
  virtual void PrintObs(const State& state, uint64_t obs, ostream& out = cout) const;
	void PrintBelief(const Belief& belief, ostream& out = cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
  void Free(State* particle) const;
	int NumActiveParticles() const;

	Belief* Tau(const Belief* belief, int action, uint64_t obs) const;
	void Observe(const Belief* belief, int action, map<uint64_t, double>& obss) const;
	double StepReward(const Belief* belief, int action) const;

	const Floor& floor() const;

	int MostLikelyAction(const vector<State*>& particles) const;

	void ComputeDefaultActions(string type) const;
	int GetAction(const State& tagstate) const;
};

#endif
