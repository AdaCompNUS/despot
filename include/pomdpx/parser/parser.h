#ifndef PARSER_H
#define PARSER_H

#include "util/tinyxml/tinyxml.h"
#include "util/util.h"
#include "core/globals.h"
#include "variable.h"
#include "function.h"

#define REWARD_VAR_VALUE "rew"
#define XML_INPUT_ERROR -1

using namespace std;

struct ValuedAction;

struct TerminalPattern {
	vector<int> state_ids;
	set<vector<int> > state_vals;
};

class Parser {
	friend class Func;

private:
	string file_name_;

	// Variables
	map<string, Variable*> variables_;

	mutable vector<StateVar> prev_state_vars_;
	mutable vector<StateVar> curr_state_vars_;
	mutable vector<ObsVar> obs_vars_;
	mutable vector<ActionVar> action_vars_;
	mutable vector<RewardVar> reward_vars_;
	mutable vector<RewardVar> terminal_reward_vars_;

	// Terminal states
	vector<TerminalPattern> terminal_state_patterns_;

	// Functions
	vector<TabularCPT> initial_belief_funcs_;
	vector<CPT*> transition_funcs_;
	vector<CPT*> obs_funcs_;
	vector<Function> reward_funcs_;
	vector<Function> terminal_reward_funcs_;

	// Noisy transition, used in particle filter
	vector<CPT*> noisy_transition_funcs_;

	// Optimizations
	bool has_terminal_;
	/* Cache self-looping states*/
	bool enable_selfloop_cache_;
	mutable set<vector<int> > selfloop_state_set_;
	mutable queue<vector<int> > selfloop_state_queue_;

	vector<vector<bool> > is_identity_; // is_identity_[s][a] = true iff transition_funcs_[s] is an identity for action a

	// Convenience and sanity-check functions for POMDPX
	TiXmlElement* GetParameterElement(TiXmlElement* func_element);
	inline string GetFirstChildText(TiXmlElement* elem,
		const char* child) const {
		return elem->FirstChildElement(child)->GetText();
	}
	inline TiXmlElement* GetFirstChildElement(TiXmlElement* elem,
		const char* child) const {
		return elem->FirstChildElement(child);
	}
	void Ensure(bool condition, string message, TiXmlBase* base = NULL) const;
	// void EnsureIdentityIsValid(TiXmlBase* base, vector<string> tokens) const; // TODO
	int ComputeNumOfEntries(const vector<string>& instance,
		const vector<NamedVar*>& parents, const NamedVar* child) const;

	// Functions for parsing the components in the POMDPX file
	void Parse(string fn);
	void ParseDiscountTag(TiXmlHandle& hDoc);
	void ParseHorizonTag(TiXmlHandle& hDoc);
	void ParseHasTerminalTag(TiXmlHandle& hDoc);

	void ParseVariableTag(TiXmlHandle& hDoc);
	StateVar CreateStateVar(TiXmlElement* varChild);ObsVar CreateObsVar(
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

	bool IsSelfLoopingWithoutReward(const vector<int>& state) const;
	bool IsInTerminalStateSet(const vector<int>& state) const;

private:
	Parser(const Parser&);
	Parser& operator=(const Parser& parser);

public:
	Parser();
	Parser(string fn);
	~Parser();

	/*
	// NOTE: disabled C++11 feature
	// Disable copy and assignment: Default copy and assignment will not work due
	// to the use of variable pointers in the functions.
	Parser(const Parser&) = delete;
	Parser& operator=(const Parser& parser) = delete;
	*/

	vector<int> CreateStateUniformly() const;
	vector<int> ComputeState(double random) const;
	vector<int> ComputeState(int index) const;
	int ComputeIndex(const vector<int>& state) const;
	double InitialWeight(const vector<int>& state) const;

	bool Step(vector<int>& state, double random, int action, double& reward,
		OBS_TYPE& obs) const;

	double GetReward(const vector<int>& perv_state,
		const vector<int>& curr_state, int action) const;
	double GetReward(int action) const;
	void GetNextState(vector<int>& state, int action, double& random) const;
	void GetNoisyNextState(vector<int>& state, int action,
		double& random) const;
	OBS_TYPE GetObservation(const vector<int>& state, int action,
		double& random) const;
	inline bool IsTerminalState(const vector<int>& state) const {
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
	double ObsProb(OBS_TYPE obs, const vector<int>& state, int action) const;
	vector<pair<vector<int>, double> > ComputeTopTransitions(
		const vector<int>& state, int action, int num) const;

	void PrintState(const vector<int>& state, ostream& out = cout) const;
	void PrintObs(OBS_TYPE obs, ostream& out = cout) const;
	void PrintAction(int action, ostream& out = cout) const;

	inline int GetDefaultAction(const vector<int>& state) const {
		return 0;
	} // TODO
	vector<int> GetDefaultActions();
	vector<int> GetUpperBounds();

	// Find the action maximizing \sum_{r} \max_{s} R_r(s, a)
	ValuedAction ComputeMaxRewardAction() const;
	// Find the action maximizing \sum_{r} \min_{s} R_r(s, a)
	ValuedAction ComputeMinRewardAction() const;

	OBS_TYPE GetPOMDPXObservation(map<string, string>& observe);
	const string& GetActionName();
	const string& GetEnumedAction(int action);

	void Print(ostream& out = cout) const;
	friend ostream& operator<<(ostream& out, const Parser& parser);
	void Check();
};

#endif
