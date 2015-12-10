#ifndef PARSER_H
#define PARSER_H

#include <despot/util/util.h>
#include <despot/core/globals.h>
#include <despot/pomdpx/parser/variable.h>
#include <despot/pomdpx/parser/function.h>

namespace despot {

namespace util {
namespace tinyxml {

class TiXmlBase;
class TiXmlElement;
class TiXmlHandle;

} // namespace tinyxml 
} // namespace util

#define REWARD_VAR_VALUE "rew"
#define XML_INPUT_ERROR -1

struct ValuedAction;

struct TerminalPattern {
  std::vector<int> state_ids;
  std::set<std::vector<int> > state_vals;
};

class Parser {
	friend class Func;

private:
  typedef util::tinyxml::TiXmlBase TiXmlBase;
  typedef util::tinyxml::TiXmlElement TiXmlElement;
  typedef util::tinyxml::TiXmlHandle TiXmlHandle;

  std::string file_name_;

	// Variables
  std::map<std::string, Variable*> variables_;

	mutable std::vector<StateVar> prev_state_vars_;
	mutable std::vector<StateVar> curr_state_vars_;
	mutable std::vector<ObsVar> obs_vars_;
	mutable std::vector<ActionVar> action_vars_;
	mutable std::vector<RewardVar> reward_vars_;
	mutable std::vector<RewardVar> terminal_reward_vars_;

	// Terminal states
  std::vector<TerminalPattern> terminal_state_patterns_;

	// Functions
  std::vector<TabularCPT> initial_belief_funcs_;
	std::vector<CPT*> transition_funcs_;
	std::vector<CPT*> obs_funcs_;
	std::vector<Function> reward_funcs_;
	std::vector<Function> terminal_reward_funcs_;

	// Noisy transition, used in particle filter
  std::vector<CPT*> noisy_transition_funcs_;

	// Optimizations
	bool has_terminal_;
	/* Cache self-looping states*/
	bool enable_selfloop_cache_;
	mutable std::set<std::vector<int> > selfloop_state_set_;
	mutable std::queue<std::vector<int> > selfloop_state_queue_;

	std::vector<std::vector<bool> > is_identity_; // is_identity_[s][a] = true iff transition_funcs_[s] is an identity for action a

	// Convenience and sanity-check functions for POMDPX
	TiXmlElement* GetParameterElement(TiXmlElement* func_element);
	inline std::string GetFirstChildText(TiXmlElement* elem,
    const char* child) const;
	inline TiXmlElement* GetFirstChildElement(TiXmlElement* elem,
		const char* child) const;

	void Ensure(bool condition, std::string message, TiXmlBase* base = NULL) const;
	// void EnsureIdentityIsValid(TiXmlBase* base, std::vector<std::string> tokens) const; // TODO
	int ComputeNumOfEntries(const std::vector<std::string>& instance,
		const std::vector<NamedVar*>& parents, const NamedVar* child) const;

	// Functions for parsing the components in the POMDPX file
	void Parse(std::string fn);
	void ParseDiscountTag(TiXmlHandle& hDoc);
	void ParseHorizonTag(TiXmlHandle& hDoc);
	void ParseHasTerminalTag(TiXmlHandle& hDoc);

	void ParseVariableTag(TiXmlHandle& hDoc);
	StateVar CreateStateVar(TiXmlElement* varChild);
  ObsVar CreateObsVar(
		TiXmlElement* varChild);ActionVar CreateActionVar(
		TiXmlElement* varChild);RewardVar CreateRewardVar(
		TiXmlElement* varChild);

	void ParseTerminalStateTag(TiXmlHandle& hDoc);

	void ParseInitialBeliefTag(TiXmlHandle& hDoc);
	TabularCPT CreateInitialBelief(TiXmlElement* func_element);

	void ParseStateTransitionTag(TiXmlHandle& hDoc);
	CPT* CreateStateTransition(TiXmlElement* func_element);
	TabularCPT* CreateTabularStateTransition(TiXmlElement* func_element);
	HierarchyCPT* CreateHierarchyStateTransition(TiXmlElement* func_element);

	void ParseObsFunctionTag(TiXmlHandle& hDoc);
	CPT* CreateObsFunction(TiXmlElement* func_element);
	TabularCPT* CreateTabularObsFunction(TiXmlElement* func_element);
	HierarchyCPT* CreateHierarchyObsFunction(TiXmlElement* func_element);

	void ParseRewardFunctionTag(TiXmlHandle& hDoc);
	Function CreateRewardFunction(TiXmlElement* func_element);

	// void ParseTerminalStateRewardTag(TiXmlHandle& hDoc); // TODO

	// void ReadBounds(); // TODO: user-specified upper bound and default action

	bool IsSelfLoopingWithoutReward(const std::vector<int>& state) const;
	bool IsInTerminalStateSet(const std::vector<int>& state) const;

private:
	Parser(const Parser&);
	Parser& operator=(const Parser& parser);

public:
	Parser();
	Parser(std::string fn);
	~Parser();

	/*
	// NOTE: disabled C++11 feature
	// Disable copy and assignment: Default copy and assignment will not work due
	// to the use of variable pointers in the functions.
	Parser(const Parser&) = delete;
	Parser& operator=(const Parser& parser) = delete;
	*/

	std::vector<int> CreateStateUniformly() const;
	std::vector<int> ComputeState(double random) const;
	std::vector<int> ComputeState(int index) const;
	int ComputeIndex(const std::vector<int>& state) const;
	double InitialWeight(const std::vector<int>& state) const;

	bool Step(std::vector<int>& state, double random, int action, double& reward,
		OBS_TYPE& obs) const;

	double GetReward(const std::vector<int>& perv_state,
		const std::vector<int>& curr_state, int action) const;
	double GetReward(int action) const;
	void GetNextState(std::vector<int>& state, int action, double& random) const;
	void GetNoisyNextState(std::vector<int>& state, int action,
		double& random) const;
	OBS_TYPE GetObservation(const std::vector<int>& state, int action,
		double& random) const;
	inline bool IsTerminalState(const std::vector<int>& state) const {
		return has_terminal_
			&& (IsInTerminalStateSet(state) || IsSelfLoopingWithoutReward(state));
	}

	inline int NumActions() const {
		return action_vars_[0].Size();
	}
	double LogNumStates() const;
	int NumStates() const;
	double LogNumInitialStates() const;
	int NumInitialStates() const;
	double LogNumObservations() const;
	OBS_TYPE NumObservations() const;
	double ObsProb(OBS_TYPE obs, const std::vector<int>& state, int action) const;
	std::vector<std::pair<std::vector<int>, double> > ComputeTopTransitions(
		const std::vector<int>& state, int action, int num) const;

	void PrintState(const std::vector<int>& state, std::ostream& out = std::cout) const;
	void PrintObs(OBS_TYPE obs, std::ostream& out = std::cout) const;
	void PrintAction(int action, std::ostream& out = std::cout) const;

	inline int GetDefaultAction(const std::vector<int>& state) const {
		return 0;
	} // TODO
	std::vector<int> GetDefaultActions();
	std::vector<int> GetUpperBounds();

	// Find the action maximizing \sum_{r} \max_{s} R_r(s, a)
	ValuedAction ComputeMaxRewardAction() const;
	// Find the action maximizing \sum_{r} \min_{s} R_r(s, a)
	ValuedAction ComputeMinRewardAction() const;

	OBS_TYPE GetPOMDPXObservation(std::map<std::string, std::string>& observe);
	const std::string& GetActionName();
	const std::string& GetEnumedAction(int action);

	void Print(std::ostream& out = std::cout) const;
	friend std::ostream& operator<<(std::ostream& out, const Parser& parser);
	void Check();
};

} // namespace despot

#endif
