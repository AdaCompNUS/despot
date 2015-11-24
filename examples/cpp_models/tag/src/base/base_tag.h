#ifndef BASETAG_H
#define BASETAG_H

#include <despot/core/pomdp.h>
#include <despot/core/mdp.h>
#include <despot/util/coord.h>
#include <despot/util/floor.h>

namespace despot {

/* ==============================================================================
 * TagState class
 * ==============================================================================*/

class TagState: public State {
public:
	TagState();
	TagState(int _state_id);

	std::string text() const;
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

	std::vector<TagState*> states_;
	std::vector<int> rob_; // rob_[s]: robot cell index for state s
	std::vector<int> opp_; // opp_[s]: opponent cell index for state s

	std::vector<std::vector<std::vector<State> > > transition_probabilities_; //state, action, [state, weight]
	OBS_TYPE same_loc_obs_;


	mutable MemoryPool<TagState> memory_pool_;

protected:
  std::map<int, double> OppTransitionDistribution(int state) const;

	void ReadConfig(std::istream& is);
	void Init(std::istream& is);
	Coord MostLikelyOpponentPosition(const std::vector<State*>& particles) const;
	Coord MostLikelyRobPosition(const std::vector<State*>& particles) const;
	const TagState& MostLikelyState(const std::vector<State*>& particles) const;
	const State* GetMMAP(const std::vector<State*>& particles) const;
	void PrintTransitions() const;

protected:
	std::string RandomMap(int height, int width, int obstacles);
	int NextRobPosition(int rob, int a) const;

	mutable std::vector<int> default_action_;

public:
  bool robot_pos_unknown_;
	static BaseTag* current_;

	BaseTag();
	BaseTag(std::string params_file);
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
	const std::vector<State>& TransitionProbability(int s, int a) const;
	double Reward(int s, int a) const;

	State* CreateStartState(std::string type = "DEFAULT") const;
	virtual Belief* InitialBelief(const State* start, std::string type = "DEFAULT") const = 0;

	inline double GetMaxReward() const {
		return TAG_REWARD;
	}
	ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;
	ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;
	BeliefUpperBound* CreateBeliefUpperBound(std::string name = "DEFAULT") const;

	inline ValuedAction GetMinRewardAction() const {
		return ValuedAction(0, -1);
	}
	ScenarioLowerBound* CreateScenarioLowerBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;
	BeliefLowerBound* CreateBeliefLowerBound(std::string name = "DEFAULT") const;

	POMCPPrior* CreatePOMCPPrior(std::string name = "DEFAULT") const;

	void PrintState(const State& state, std::ostream& out = std::cout) const;
	void PrintBelief(const Belief& belief, std::ostream& out = std::cout) const;
	virtual void PrintObs(const State& state, OBS_TYPE obs, std::ostream& out = std::cout) const = 0;
	void PrintAction(int action, std::ostream& out = std::cout) const;

	State* Allocate(int state_id, double weight) const;
	State* Copy(const State* particle) const;
	void Free(State* particle) const;
	int NumActiveParticles() const;

	const Floor& floor() const;

	int MostLikelyAction(const std::vector<State*>& particles) const;

	void ComputeDefaultActions(std::string type) const;
	int GetAction(const State& tagstate) const;

	Belief* Tau(const Belief* belief, int action, OBS_TYPE obs) const;
	void Observe(const Belief* belief, int action, std::map<OBS_TYPE, double>& obss) const = 0;
	double StepReward(const Belief* belief, int action) const;
};

/* ==============================================================================
 * TagBelief class
 * ==============================================================================*/

class TagBelief: public ParticleBelief {
private:
	const BaseTag* tag_model_;
public:
	TagBelief(std::vector<State*> particles, const BaseTag* model, Belief* prior =
		NULL);
	void Update(int action, OBS_TYPE obs);
};

} // namespace despot

#endif
