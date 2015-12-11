#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <despot/util/random.h>
#include <despot/util/tinyxml/tinyxml.h>
#include <despot/pomdpx/parser/parser.h>
#include <despot/core/pomdp.h>

using namespace std;
using namespace despot::util::tinyxml;

namespace despot {

Parser::Parser(const Parser&) {
}

Parser& Parser::operator=(const Parser& parser) {
	return *this;
}

Parser::Parser() :
	enable_selfloop_cache_(true) {
}

Parser::Parser(string fn) :
	enable_selfloop_cache_(true) {
	Parse(fn);
	// Check();
}

Parser::~Parser() {
}

ostream& operator<<(ostream& out, const Parser& parser) {
	parser.Print(out);
	return out;
}

void Parser::Print(ostream& out) const {
	out << "State variables" << endl;
	for (int s = 0; s < curr_state_vars_.size(); s++) {
		out << "s[" << s << "] = " << curr_state_vars_[s] << endl;
	}
	out << endl;

	out << "Observation variables" << endl;
	for (int o = 0; o < obs_vars_.size(); o++) {
		out << "o[" << o << "] = " << obs_vars_[o] << endl;
	}
	out << endl;

	out << "# Initial belief" << endl;
	for (int s = 0; s < initial_belief_funcs_.size(); s++) {
		out << "s[" << s << "] = " << initial_belief_funcs_[s] << endl;
	}
	out << endl;

	out << "# Transition probabilities" << endl;
	for (int s = 0; s < transition_funcs_.size(); s++) {
		out << "s[" << s << "] = " << *transition_funcs_[s] << endl;
	}
	out << endl;

	out << "# Observation probabilities" << endl;
	for (int o = 0; o < obs_funcs_.size(); o++) {
		out << "o[" << o << "] = " << obs_funcs_[o] << endl;
	}
	out << endl;

	out << "# Reward functions" << endl;
	for (int r = 0; r < reward_funcs_.size(); r++) {
		out << "r[" << r << "] = " << reward_funcs_[r] << endl;
	}
	out << endl;
}

void Parser::Check() {
	cout << *this << endl;

	cout << "# Terminal states:" << endl;
	for (int s = 0; s < NumStates(); s++) {
		vector<int> state = ComputeState(s);
		if (IsTerminalState(state))
			PrintState(state);
	}
	cout << endl;

	double rand;
	int action;
	OBS_TYPE obs;
	for (int i = 0; i < 20; i++) {
		rand = Random::RANDOM.NextDouble();
		vector<int> state = ComputeState(rand);
		cout << "s = " << state << " - " << rand << endl;
		PrintState(state);

		for (int j = 0; j < 2; j++) {
			action = Random::RANDOM.NextInt(action_vars_[0].Size());
			cout << "a = " << action << endl;
			PrintAction(action);
			rand = Random::RANDOM.NextDouble();
			GetNextState(state, action, rand);
			cout << "s' = " << state << " - " << rand << endl;
			PrintState(state);
			cout << "r = " << GetReward(action) << endl;
			rand = Random::RANDOM.NextDouble();
			obs = GetObservation(state, action, rand);
			cout << "o = " << obs << " - " << rand << " "
				<< ObsProb(obs, state, action) << endl;
			PrintObs(obs);
			cout << endl;
		}

		cout << "----------------------------------" << endl;
	}
	exit(0);
}

ValuedAction Parser::ComputeMaxRewardAction() const {
	ValuedAction max(-1, Globals::NEG_INFTY);
	for (int a = 0; a < action_vars_[0].Size(); a++) {
		double reward = 0;
		for (int r = 0; r < reward_funcs_.size(); r++) {
			reward += reward_funcs_[r].ComputeConstrainedMaximum(
				&action_vars_[0], a);
		}
		if (reward > max.value) {
			max = ValuedAction(a, reward);
		}
	}
	return max;
}

ValuedAction Parser::ComputeMinRewardAction() const {
	ValuedAction min(-1, Globals::NEG_INFTY);
	for (int a = 0; a < action_vars_[0].Size(); a++) {
		double reward = 0;
		for (int r = 0; r < reward_funcs_.size(); r++) {
			reward += reward_funcs_[r].ComputeConstrainedMinimum(
				&action_vars_[0], a);
		}
		if (reward > min.value) {
			min = ValuedAction(a, reward);
		}
	}
	return min;
}

void Parser::Parse(string fn) {
	file_name_ = fn;

	clock_t start = clock();
	TiXmlDocument doc(file_name_.c_str());

	// Load file
	logi << "Loading file " << file_name_ << "...";
	bool loadOkay = doc.LoadFile();
	logi << "Done [" << double(clock() - start) / CLOCKS_PER_SEC << "s]"
		<< endl;

	Ensure(loadOkay,
		"Could not load pomdpX file.\n  Line" + to_string(doc.ErrorRow()) + ":"
			+ string(doc.ErrorDesc()) + "\n"
			+ "  Check pomdpX file with pomdpX's XML schema using an XML validator.\n");

	TiXmlHandle xml_handle(&doc);
	TiXmlElement *elem = xml_handle.FirstChildElement().ToElement();
	string msg = elem->Value();

	start = clock();
	// Parse all the tags
	ParseDiscountTag(xml_handle);
	ParseHorizonTag(xml_handle);
	ParseHasTerminalTag(xml_handle);
	ParseVariableTag(xml_handle);
	logi << "Parsing terminal tag...";
	ParseTerminalStateTag(xml_handle);
	logi << "Done." << endl;
	logi << "Parsing initial belief...";
	logi.flush();
	ParseInitialBeliefTag(xml_handle);
	logi << "Done." << endl;
	logi << "Parsing transitions...";
	logi.flush();
	ParseStateTransitionTag(xml_handle);
	logi << "Done." << endl;
	logi << "Parsing obs function...";
	logi.flush();
	ParseObsFunctionTag(xml_handle);
	logi << "Done." << endl;
	logi << "Parsing reward function...";
	logi.flush();
	ParseRewardFunctionTag(xml_handle);
	logi << "Done." << endl;
	// cout << "Parsing terminal state reward" << endl;
	// ParseTerminalStateRewardTag(xml_handle);
	// ReadBounds();
	logi << "Parsing done in " << double(clock() - start) / CLOCKS_PER_SEC
		<< "s" << endl;
}

void Parser::ParseDiscountTag(TiXmlHandle& xml_handle) {
	TiXmlElement* e_Discount = xml_handle.FirstChild("pomdpx").FirstChild(
		"Discount").ToElement();

	Ensure(e_Discount != NULL, "Cannot find Discount tag");

	string discount_str = e_Discount->GetText();
	Globals::config.discount = atof(discount_str.c_str());
	logi << "Discount = " << Globals::config.discount << endl;

	Ensure(Globals::config.discount < 1.0 && Globals::config.discount > 0,
		"Discount factor must be in (0, 1).", e_Discount);
}

void Parser::ParseHorizonTag(TiXmlHandle& xml_handle) {
	TiXmlElement* e_Horizon = xml_handle.FirstChild("pomdpx").FirstChild(
		"Horizon").ToElement();
	if (e_Horizon != NULL) {
		string sim_len_str = e_Horizon->GetText();
		Globals::config.sim_len = atoi(sim_len_str.c_str());
		Globals::config.search_depth = atoi(sim_len_str.c_str());
	}
	logi << "Horizon = " << Globals::config.sim_len << endl;
}

void Parser::ParseHasTerminalTag(TiXmlHandle& xml_handle) {
	TiXmlElement* e_HasTerminal = xml_handle.FirstChild("pomdpx").FirstChild(
		"HasTerminal").ToElement();
	if (e_HasTerminal != NULL) {
		has_terminal_ = (lower(e_HasTerminal->GetText()) == "true");
	} else {
		has_terminal_ = true;
	}
	logi << "HasTerminal = " << has_terminal_ << endl;
}

void Parser::ParseVariableTag(TiXmlHandle& xml_handle) {
	TiXmlElement* e_Variable = xml_handle.FirstChild("pomdpx").FirstChild(
		"Variable").ToElement();

	Ensure(e_Variable != NULL, "Cannot find Variable tag");

	TiXmlElement* e_Var = e_Variable->FirstChildElement();
	while (e_Var != 0) {
		string type = e_Var->Value();
		if (type == "StateVar") {
			StateVar prev_state = CreateStateVar(e_Var);
			prev_state.name(prev_state.prev_name());
			prev_state_vars_.push_back(prev_state);

			StateVar curr_state = prev_state;
			curr_state.name(prev_state.curr_name());
			curr_state_vars_.push_back(curr_state);
		} else if (type == "ObsVar") {
			ObsVar obs = CreateObsVar(e_Var);
			obs_vars_.push_back(obs);
		} else if (type == "ActionVar") {
			Ensure(action_vars_.size() == 0,
				"Only one action variable is allowed!");

			ActionVar act = CreateActionVar(e_Var);
			action_vars_.push_back(act);
		} else if (type == "RewardVar") {
			RewardVar reward = CreateRewardVar(e_Var);
			reward_vars_.push_back(reward);
		} else {
			Ensure(false, "Unknown XML tag: " + type + " encountered", e_Var);
		}

		e_Var = e_Var->NextSiblingElement();
	}

	for (int i = 0; i < prev_state_vars_.size(); i++) {
		StateVar& var = prev_state_vars_[i];
		variables_[var.name()] = &var;
	}
	for (int i = 0; i < curr_state_vars_.size(); i++) {
		StateVar& var = curr_state_vars_[i];
		variables_[var.name()] = &var;
	}

	for (int i = 0; i < obs_vars_.size(); i++) {
		ObsVar& var = obs_vars_[i];
		variables_[var.name()] = &var;
	}

	for (int i = 0; i < action_vars_.size(); i++) {
		ActionVar& var = action_vars_[i];
		variables_[var.name()] = &var;
	}

	for (int i = 0; i < reward_vars_.size(); i++) {
		RewardVar& var = reward_vars_[i];
		variables_[var.name()] = &var;
	}
}

StateVar Parser::CreateStateVar(TiXmlElement* e_StateVar) {
	StateVar state;

	// vnamePrev
	string prev_name = e_StateVar->Attribute("vnamePrev");
	state.prev_name(prev_name);

	// vnameCurr
	string curr_name = e_StateVar->Attribute("vnameCurr");
	state.curr_name(curr_name);

	// fullyObs
	const char* fully_observed;
	if (e_StateVar->Attribute("fullyObs"))
		fully_observed = e_StateVar->Attribute("fullyObs");
	else
		fully_observed = "false"; //default is false
	state.observed(strcmp(fully_observed, "true") == 0);

	// ValueEnum or NumValues
	if (e_StateVar->FirstChild("ValueEnum")) {
		string value_enum_list = GetFirstChildText(e_StateVar, "ValueEnum");
		vector<string> values = Tokenize(value_enum_list);
		state.values(values);
	} else {
		int num = atoi(GetFirstChildText(e_StateVar, "NumValues").c_str());
		state.values("s", num);
	}

	return state;
}

ObsVar Parser::CreateObsVar(TiXmlElement* e_ObsVar) {
	ObsVar obs;

	// <vname>
	string name = e_ObsVar->Attribute("vname");
	obs.name(name);

	// <ValueEnum> or <NumValues>
	if (e_ObsVar->FirstChild("ValueEnum")) {
		string value_enum_list = GetFirstChildText(e_ObsVar, "ValueEnum");
		vector<string> values = Tokenize(value_enum_list);
		obs.values(values);
	} else {
		int num = atoi(GetFirstChildText(e_ObsVar, "NumValues").c_str());
		obs.values("o", num);
	}

	return obs;
}

RewardVar Parser::CreateRewardVar(TiXmlElement* e_RewardVar) {
	RewardVar rew(e_RewardVar->Attribute("vname"));

	vector<string> rewardEnum;
	rewardEnum.push_back(REWARD_VAR_VALUE);
	rew.values(rewardEnum);

	return rew;
}

ActionVar Parser::CreateActionVar(TiXmlElement* e_ActionVar) {
	ActionVar act(e_ActionVar->Attribute("vname"));

	if (e_ActionVar->FirstChild("ValueEnum")) {
		string value_enum_list = GetFirstChildText(e_ActionVar, "ValueEnum");
		vector<string> tokens = Tokenize(value_enum_list);
		act.values(tokens);
	} else {
		int num = atoi(GetFirstChildText(e_ActionVar, "NumValues").c_str());
		act.values("a", num);
	}

	return act;
}

void Parser::ParseTerminalStateTag(TiXmlHandle& handle) {
	TiXmlElement* e_Terminal = handle.FirstChild("pomdpx").FirstChild(
		"TerminalState").ToElement();

	if (e_Terminal == NULL)
		return;

	// <Table>
	TiXmlElement* e_Table = GetFirstChildElement(e_Terminal, "Table");
	while (e_Table != NULL) {
		TerminalPattern pattern;
		// <Vars>
		vector<string> names = Tokenize(GetFirstChildText(e_Table, "Vars"));
		for (int i = 0; i < names.size(); i++) {
			string name = names[i];
			int id = 0;
			while (curr_state_vars_[id].name() != name)
				id++;
			pattern.state_ids.push_back(id);
		}

		TiXmlElement* e_Instance = e_Table->FirstChildElement("Instance");
		while (e_Instance != NULL) {
			// <Instance>
			vector<string> instance = Tokenize(e_Instance->GetText());
			vector<int> values;
			for (int i = 0; i < instance.size(); i++) {
				values.push_back(
					curr_state_vars_[pattern.state_ids[i]].IndexOf(
						instance[i]));
			}
			pattern.state_vals.insert(values);

			e_Instance = e_Instance->NextSiblingElement();
		}

		terminal_state_patterns_.push_back(pattern);

		e_Table = e_Table->NextSiblingElement();
	}
}

void Parser::ParseInitialBeliefTag(TiXmlHandle& handle) {
	TiXmlElement* e_CondProb = handle.FirstChild("pomdpx").FirstChild(
		"InitialStateBelief").FirstChild("CondProb").ToElement();

	while (e_CondProb != NULL) {
		TabularCPT belief_function = CreateInitialBelief(e_CondProb);
		belief_function.ComputeSparseChildDistribution();

		Ensure(belief_function.Validate(),
			"In <InitialStateBelief>: Probability distribution does not sum to one!",
			e_CondProb);

		initial_belief_funcs_.push_back(belief_function);

		e_CondProb = e_CondProb->NextSiblingElement();
	}

	// TODO: Check whether each state variable corresponds one CondProb
}

TabularCPT Parser::CreateInitialBelief(TiXmlElement* e_CondProb) {
	// <Var>
	string name = GetFirstChildText(e_CondProb, "Var");

	Ensure(Variable::IsVariableName(name, prev_state_vars_),
		"In <InitialStateBelief>: Var " + name
			+ " is not declared as a vnamePrev within the <Variable> tag.");

	// logi << "Var name: " << name << endl;
	StateVar* child = static_cast<StateVar*>(variables_[name]);

	// <Parent>
	vector<string> parent_names = Tokenize(
		GetFirstChildText(e_CondProb, "Parent"));
	// logi << "Parents: " << parent_names << endl;

	Ensure((parent_names.size() == 1 && parent_names[0] == "null"),
		"In <InitialStateBelief>: Var " + name
			+ " must have null in the <Parent> tag.",
		GetFirstChildElement(e_CondProb, "Parent"));

	vector<NamedVar*> parents;
	TabularCPT func(child, parents);
	// logi << "Initialized" << endl;

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_CondProb);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));

		// <ProbTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ProbTable");

		int num_entries = ComputeNumOfEntries(instance, parents, child);

		if (table == "uniform") {
			int num = child->Size();
			for (int i = 0; i < num; i++) {
				values.push_back(1.0 / num);
			}
		} else {
			vector<string> value_tokens = Tokenize(table);

			Ensure(value_tokens.size() == num_entries,
				"In <InitialStateBelief>: " + to_string(num_entries)
					+ " entries expected but " + to_string(value_tokens.size())
					+ " found.", GetFirstChildElement(e_Entry, "ProbTable"));

			values.resize(value_tokens.size());
			for (int i = 0; i < value_tokens.size(); i++) {
				values[i] = atof(value_tokens[i].c_str());
			}
		}
		// logi << "instance " << instance << endl;
		// logi << "values " << values << endl;

		bool success = func.SetValue(instance, values);

		// logi << "success " << success << endl;
		Ensure(success, "In <InitialStateBelief>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

void Parser::ParseStateTransitionTag(TiXmlHandle& handle) {
	TiXmlElement* e_StateTransition = handle.FirstChild("pomdpx").FirstChild(
		"StateTransitionFunction").ToElement();

	Ensure(e_StateTransition != NULL,
		"Cannot find StateTransitionFunction tag");

	TiXmlElement* e_CondProb = e_StateTransition->FirstChildElement();
	while (e_CondProb != 0) {
		CPT* transition = CreateStateTransition(e_CondProb);
		transition->ComputeSparseChildDistribution();
		transition_funcs_.push_back(transition);

		CPT* noisy_variant =
			Globals::config.noise ?
				transition->CreateNoisyVariant(Globals::config.noise) :
				transition;
		noisy_transition_funcs_.push_back(noisy_variant);

		Ensure(transition->Validate(),
			"In <StateTransitionFunction>: Probability distribution does not sum to one!",
			e_CondProb);

		e_CondProb = e_CondProb->NextSiblingElement();
	}

	// TODO: Check whether each state variable corresponds to one CondProb

	int num_states = transition_funcs_.size(), num_actions =
		action_vars_[0].Size();
	is_identity_.resize(num_states);
	for (int s = 0; s < num_states; s++) {
		is_identity_[s].resize(num_actions);
		for (int a = 0; a < num_actions; a++) {
			is_identity_[s][a] =
				transition_funcs_[s]->IsIdentityUnderConstraint(
					&action_vars_[0], a);
		}
	}
}

CPT* Parser::CreateStateTransition(TiXmlElement* e_CondProb) {
	if (GetFirstChildElement(e_CondProb, "Dependency") == NULL)
		return CreateTabularStateTransition(e_CondProb);
	else
		return CreateHierarchyStateTransition(e_CondProb);
}

TabularCPT* Parser::CreateTabularStateTransition(TiXmlElement* e_CondProb) {
	// <Var>
	string name = GetFirstChildText(e_CondProb, "Var");

	Ensure(Variable::IsVariableName(name, curr_state_vars_),
		"In <StateTransitionFunction>: Var " + name
			+ " has not been declared as a StateVar vnameCurr within the <Variable> tag.",
		e_CondProb->FirstChild("Var"));

	StateVar* child = static_cast<StateVar*>(variables_[name]);

	// <Parent>
	vector<NamedVar*> parents;
	vector<string> parent_names = Tokenize(
		GetFirstChildText(e_CondProb, "Parent"));
	for (int i = 0; i < parent_names.size(); i++) {
		string parent_name = parent_names[i];
		Ensure(
			Variable::IsVariableName(parent_name, prev_state_vars_)
				|| Variable::IsVariableName(parent_name, curr_state_vars_)
				|| Variable::IsVariableName(parent_name, action_vars_),
			"In <StateTransitionFunction>: Parent " + parent_name
				+ " has not been declared as a StateVar vnamePrev/vnameCurr or an ActionVar vname within the <Variable> tag.",
			e_CondProb->FirstChild("Parent"));

		parents.push_back(static_cast<StateVar*>(variables_[parent_name]));
	}

	TabularCPT* func = new TabularCPT(child, parents);

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_CondProb);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));

		// <ProbTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ProbTable");

		int num_entries = ComputeNumOfEntries(instance, parents, child);

		if (table == "uniform") { // keyword uniform
			int num = child->Size();
			for (int i = 0; i < num_entries; i++) {
				values.push_back(1.0 / num);
			}
		} else if (table == "identity") { // keyword identity
			//EnsureIdentityIsValid(); // TODO
			for (int i = 0; i < child->Size(); i++) {
				for (int j = 0; j < child->Size(); j++) {
					values.push_back(i == j);
				}
			}
		} else { // numbers
			vector<string> value_tokens = Tokenize(table);

			Ensure(num_entries == value_tokens.size(),
				"In <StateTransitionFunction>: " + to_string(num_entries)
					+ " entries expected but " + to_string(value_tokens.size())
					+ " found.", GetFirstChildElement(e_Entry, "ProbTable"));

			values.resize(value_tokens.size());
			for (int i = 0; i < value_tokens.size(); i++) {
				values[i] = atof(value_tokens[i].c_str());
			}
		}

		bool success = func->SetValue(instance, values);

		Ensure(success,
			"In <StateTransitionFunction>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

HierarchyCPT* Parser::CreateHierarchyStateTransition(TiXmlElement* e_CondProb) {
	// <Var>
	string name = GetFirstChildText(e_CondProb, "Var");

	Ensure(Variable::IsVariableName(name, curr_state_vars_),
		"In <StateTransitionFunction>: Var " + name
			+ " has not been declared as a StateVar vnameCurr within the <Variable> tag.",
		e_CondProb->FirstChild("Var"));

	StateVar* child = static_cast<StateVar*>(variables_[name]);

	// <Parent>
	vector<NamedVar*> parents;
	vector<string> parent_names = Tokenize(
		GetFirstChildText(e_CondProb, "Parent"));
	for (int i = 0; i < parent_names.size(); i++) {
		string parent_name = parent_names[i];
		Ensure(
			Variable::IsVariableName(parent_name, prev_state_vars_)
				|| Variable::IsVariableName(parent_name, curr_state_vars_)
				|| Variable::IsVariableName(parent_name, action_vars_),
			"In <StateTransitionFunction>: Parent " + parent_name
				+ " has not been declared as a StateVar vnamePrev/vnameCurr or an ActionVar vname within the <Variable> tag.",
			e_CondProb->FirstChild("Parent"));

		parents.push_back(static_cast<StateVar*>(variables_[parent_name]));
	}

	HierarchyCPT* func = new HierarchyCPT(child, parents);

	TiXmlElement* e_Dependency = GetFirstChildElement(e_CondProb, "Dependency");
	TiXmlElement* e_Vars = GetFirstChildElement(e_Dependency, "Vars");
	while (e_Vars != NULL) {
		vector<string> tokens = Tokenize(e_Vars->GetText());
		int val = parents[0]->IndexOf(tokens[0]);
		vector<NamedVar*> vars;
		for (int i = 1; i < tokens.size(); i++)
			vars.push_back(static_cast<NamedVar*>(variables_[tokens[i]]));
		func->SetParents(val, vars);

		e_Vars = e_Vars->NextSiblingElement();
	}

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_CondProb);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));

		// <ProbTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ProbTable");

		vector<string> value_tokens = Tokenize(table);

		values.resize(value_tokens.size());
		for (int i = 0; i < value_tokens.size(); i++) {
			values[i] = atof(value_tokens[i].c_str());
		}

		int val = parents[0]->IndexOf(instance[0]);
		instance.erase(instance.begin());
		bool success = func->SetValue(val, instance, values);

		Ensure(success,
			"In <StateTransitionFunction>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

void Parser::ParseObsFunctionTag(TiXmlHandle& handle) {
	if (obs_vars_.size() > 0) {
		TiXmlElement* e_ObsFunction = handle.FirstChild("pomdpx").FirstChild(
			"ObsFunction").ToElement();
		Ensure(e_ObsFunction != NULL, "Cannot find ObsFunction tag");

		TiXmlElement* e_CondProb = e_ObsFunction->FirstChildElement();
		while (e_CondProb != 0) {
			CPT* obs_func = CreateObsFunction(e_CondProb);
			obs_func->ComputeSparseChildDistribution();
			obs_funcs_.push_back(obs_func);

			Ensure(obs_func->Validate(),
				"In <ObsFunction>: Probability distribution does not sum to one!",
				e_CondProb);

			e_CondProb = e_CondProb->NextSiblingElement();
		}

		// TODO: Check whether each obs variable corresponds to one CondProb
	}
}

CPT* Parser::CreateObsFunction(TiXmlElement* e_CondProb) {
	if (GetFirstChildElement(e_CondProb, "Dependency") == NULL)
		return CreateTabularObsFunction(e_CondProb);
	else
		return CreateHierarchyObsFunction(e_CondProb);
}

TabularCPT* Parser::CreateTabularObsFunction(TiXmlElement* e_CondProb) {
	// <Var>
	string name = GetFirstChildText(e_CondProb, "Var");

	Ensure(Variable::IsVariableName(name, obs_vars_),
		"In <ObsFunction>: Variable " + name
			+ " has not been declared as an ObsVar vname within the <Variable> tag.",
		e_CondProb->FirstChild("Var"));

	ObsVar* child = static_cast<ObsVar*>(variables_[name]);

	// <Parent>
	vector<NamedVar*> parents;
	vector<string> parent_names = Tokenize(
		GetFirstChildText(e_CondProb, "Parent"));
	for (int i = 0; i < parent_names.size(); i++) {
		string parent_name = parent_names[i];
		Ensure(
			Variable::IsVariableName(parent_name, curr_state_vars_)
				|| Variable::IsVariableName(parent_name, action_vars_),
			"In <ObsFunction>: Parent " + parent_name
				+ " has not been declared as a StateVar vnameCurr or an ActionVar vname within the <Variable> tag.",
			e_CondProb->FirstChild("Parent"));

		parents.push_back(static_cast<StateVar*>(variables_[parent_name]));
	}

	TabularCPT* func = new TabularCPT(child, parents);

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_CondProb);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));

		// <ProbTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ProbTable");

		int num_entries = ComputeNumOfEntries(instance, parents, child);

		if (table == "uniform") { // keyword uniform
			int num = child->Size();
			for (int i = 0; i < num_entries; i++) {
				values.push_back(1.0 / num);
			}
		} else { // numbers
			vector<string> value_tokens = Tokenize(table);

			Ensure(num_entries == value_tokens.size(),
				"In <ObsFunction>: " + to_string(num_entries)
					+ " entries expected but " + to_string(value_tokens.size())
					+ " found.", GetFirstChildElement(e_Entry, "ProbTable"));

			values.resize(value_tokens.size());
			for (int i = 0; i < value_tokens.size(); i++) {
				values[i] = atof(value_tokens[i].c_str());
			}
		}

		bool success = func->SetValue(instance, values);

		Ensure(success, "In <ObsFunction>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

HierarchyCPT* Parser::CreateHierarchyObsFunction(TiXmlElement* e_CondProb) {
	// <Var>
	string name = GetFirstChildText(e_CondProb, "Var");

	Ensure(Variable::IsVariableName(name, obs_vars_),
		"In <ObsFunction>: Variable " + name
			+ " has not been declared as an ObsVar vname within the <Variable> tag.",
		e_CondProb->FirstChild("Var"));

	ObsVar* child = static_cast<ObsVar*>(variables_[name]);

	// <Parent>
	vector<NamedVar*> parents;
	vector<string> parent_names = Tokenize(
		GetFirstChildText(e_CondProb, "Parent"));
	for (int i = 0; i < parent_names.size(); i++) {
		string parent_name = parent_names[i];
		Ensure(
			Variable::IsVariableName(parent_name, curr_state_vars_)
				|| Variable::IsVariableName(parent_name, action_vars_),
			"In <ObsFunction>: Parent " + parent_name
				+ " has not been declared as a StateVar vnameCurr or an ActionVar vname within the <Variable> tag.",
			e_CondProb->FirstChild("Parent"));

		parents.push_back(static_cast<StateVar*>(variables_[parent_name]));
	}

	HierarchyCPT* func = new HierarchyCPT(child, parents);

	TiXmlElement* e_Dependency = GetFirstChildElement(e_CondProb, "Dependency");
	TiXmlElement* e_Vars = GetFirstChildElement(e_Dependency, "Vars");
	while (e_Vars != NULL) {
		vector<string> tokens = Tokenize(e_Vars->GetText());
		int val = parents[0]->IndexOf(tokens[0]);
		vector<NamedVar*> vars;
		for (int i = 1; i < tokens.size(); i++)
			vars.push_back(static_cast<NamedVar*>(variables_[tokens[i]]));
		func->SetParents(val, vars);

		e_Vars = e_Vars->NextSiblingElement();
	}

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_CondProb);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));

		// <ProbTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ProbTable");

		vector<string> value_tokens = Tokenize(table);

		values.resize(value_tokens.size());
		for (int i = 0; i < value_tokens.size(); i++) {
			values[i] = atof(value_tokens[i].c_str());
		}

		int val = parents[0]->IndexOf(instance[0]);
		instance.erase(instance.begin());
		bool success = func->SetValue(val, instance, values);

		Ensure(success, "In <ObsFunction>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

void Parser::ParseRewardFunctionTag(TiXmlHandle& handle) {
	TiXmlElement* e_RewardFunction = handle.FirstChild("pomdpx").FirstChild(
		"RewardFunction").ToElement();

	Ensure(e_RewardFunction != NULL, "Cannot find RewardFunction tag");

	TiXmlElement* e_Func = e_RewardFunction->FirstChildElement();

	while (e_Func != 0) {
		Function reward_func = CreateRewardFunction(e_Func);
		reward_funcs_.push_back(reward_func);

		e_Func = e_Func->NextSiblingElement();
	}

	// TODO: incomplete check
	Ensure(reward_funcs_.size() == reward_vars_.size(),
		"Each reward variable should correspond to one reward Functions",
		e_RewardFunction);
}

Function Parser::CreateRewardFunction(TiXmlElement* e_Func) {
	// <Var>
	string name = GetFirstChildText(e_Func, "Var");

	Ensure(Variable::IsVariableName(name, reward_vars_),
		"In <RewardFunction>: Variable " + name
			+ " has not been declared as a RewardVar vname within <Variable> tag.",
		e_Func->FirstChild("Var"));

	ObsVar* child = static_cast<ObsVar*>(variables_[name]);

	// <Parent>
	vector<NamedVar*> parents;
	vector<string> parent_names = Tokenize(GetFirstChildText(e_Func, "Parent"));
	for (int i = 0; i < parent_names.size(); i++) {
		string str = parent_names[i];
		parents.push_back(static_cast<StateVar*>(variables_[str]));
	}

	Function func(child, parents);

	// <Parameter>
	TiXmlElement* e_Parameter = GetParameterElement(e_Func);
	TiXmlElement* e_Entry = e_Parameter->FirstChildElement("Entry");
	while (e_Entry != NULL) {
		// <Instance>
		vector<string> instance = Tokenize(
			GetFirstChildText(e_Entry, "Instance"));
		instance.push_back(REWARD_VAR_VALUE);

		// <ValueTable>
		vector<double> values;
		string table = GetFirstChildText(e_Entry, "ValueTable");

		int num_entries = ComputeNumOfEntries(instance, parents, child);

		vector<string> value_tokens = Tokenize(table);

		Ensure(num_entries == value_tokens.size(),
			"In <RewardFunction>: " + to_string(num_entries)
				+ " entries expected but " + to_string(value_tokens.size())
				+ " found.", GetFirstChildElement(e_Entry, "ValueTable"));

		values.resize(value_tokens.size());
		for (int i = 0; i < value_tokens.size(); i++) {
			values[i] = atof(value_tokens[i].c_str());
		}

		bool success = func.SetValue(instance, values);

		Ensure(success, "In <RewardFunction>: Value assignment failed.",
			GetFirstChildElement(e_Entry, "Instance"));

		e_Entry = e_Entry->NextSiblingElement();
	}

	return func;
}

int Parser::ComputeNumOfEntries(const vector<string>& instance,
	const vector<NamedVar*>& parents, const NamedVar* child) const {
	int num_entries = 1;
	for (int i = 0; i < instance.size(); i++) {
		if (instance[i] == "-") {
			if (i < instance.size() - 1)
				num_entries *= parents[i]->Size();
			else
				num_entries *= child->Size();
		}
	}
	return num_entries;
}

void Parser::Ensure(bool condition, string message, TiXmlBase* base) const {
	if (!condition) {
		cerr << "ERROR: ";
		if (base != NULL)
			cerr << file_name_ << ":" << base->Row() << ":" << endl << " ";
		cerr << message << endl;
		exit(XML_INPUT_ERROR);
	}
}

TiXmlElement* Parser::GetParameterElement(TiXmlElement* element) {
	TiXmlElement* e_Parameter = element->FirstChildElement("Parameter");

	string param_type = "";
	if (e_Parameter->Attribute("type") == NULL)
		param_type = "TBL"; //the default case
	else
		param_type = e_Parameter->Attribute("type");

	Ensure(param_type == "TBL", "Only parameter type \"TBL\" is supported.\n");

	return e_Parameter;
}

string Parser::GetFirstChildText(TiXmlElement* elem,
    const char* child) const {
  return elem->FirstChildElement(child)->GetText();
}

TiXmlElement* Parser::GetFirstChildElement(TiXmlElement* elem,
    const char* child) const {
  return elem->FirstChildElement(child);
}

vector<int> Parser::CreateStateUniformly() const {
	vector<int> state(curr_state_vars_.size());
	for (int s = 0; s < state.size(); s++) {
		state[s] = Random::RANDOM.NextInt(curr_state_vars_[s].Size());
	}
	return state;
}

vector<int> Parser::ComputeState(double random) const {
	vector<int> state(curr_state_vars_.size());
	for (int s = 0; s < state.size(); s++) {
		state[s] = initial_belief_funcs_[s].ComputeCurrentIndex(random);
	}

	return state;
}

vector<int> Parser::ComputeState(int index) const {
	vector<int> state(curr_state_vars_.size());
	for (int s = state.size() - 1; s >= 0; s--) {
		state[s] = index % curr_state_vars_[s].Size();
		index /= curr_state_vars_[s].Size();
	}

	return state;
}

int Parser::ComputeIndex(const vector<int>& state) const {
	int index = 0;
	for (int s = 0; s < curr_state_vars_.size(); s++) {
		index = index * curr_state_vars_[s].Size() + state[s];
	}
	return index;
}

double Parser::InitialWeight(const vector<int>& state) const {
	double prob = 1;
	for (int s = 0; s < curr_state_vars_.size(); s++) {
		prob *= initial_belief_funcs_[s].GetValue(0, state[s]);
	}
	return prob;
}

bool Parser::Step(vector<int>& state, double random, int action, double& reward,
	OBS_TYPE& obs) const {
	for (int s = 0; s < prev_state_vars_.size(); s++)
		prev_state_vars_[s].curr_value = state[s];
	action_vars_[0].curr_value = action;

	for (int s = 0; s < state.size(); s++) {
		state[s] = transition_funcs_[s]->ComputeCurrentIndex(random);
	}

	for (int s = 0; s < curr_state_vars_.size(); s++)
		curr_state_vars_[s].curr_value = state[s];

	// Reward may depend on both current state and prev state
	reward = 0;
	for (int r = 0; r < reward_funcs_.size(); r++)
		reward += reward_funcs_[r].GetValue();

	obs = 0;
	for (int o = 0; o < obs_vars_.size(); o++) {
		obs = obs * obs_vars_[o].Size()
			+ obs_funcs_[o]->ComputeCurrentIndex(random);
	}

	return IsTerminalState(state);
}

double Parser::GetReward(const vector<int>& prev_state,
	const vector<int>& curr_state, int action) const {
	for (int s = 0; s < prev_state_vars_.size(); s++)
		prev_state_vars_[s].curr_value = prev_state[s];
	for (int s = 0; s < curr_state_vars_.size(); s++)
		curr_state_vars_[s].curr_value = curr_state[s];
	action_vars_[0].curr_value = action;

	double reward = 0;
	for (int r = 0; r < reward_funcs_.size(); r++)
		reward += reward_funcs_[r].GetValue();
	return reward;
}

double Parser::GetReward(int action) const {
	action_vars_[0].curr_value = action;

	double reward = 0;
	for (int r = 0; r < reward_funcs_.size(); r++)
		reward += reward_funcs_[r].GetValue();
	return reward;
}
void Parser::GetNextState(vector<int>& state, int action,
	double& random) const {
	for (int s = 0; s < state.size(); s++) {
		prev_state_vars_[s].curr_value = state[s];
	}
	action_vars_[0].curr_value = action;

	for (int s = 0; s < state.size(); s++) {
		if (!is_identity_[s][action])
			state[s] = transition_funcs_[s]->ComputeCurrentIndex(random);
		curr_state_vars_[s].curr_value = state[s];
	}
}

void Parser::GetNoisyNextState(vector<int>& state, int action,
	double& random) const {
	for (int s = 0; s < state.size(); s++) {
		prev_state_vars_[s].curr_value = state[s];
	}
	action_vars_[0].curr_value = action;

	for (int s = 0; s < state.size(); s++) {
		if (!is_identity_[s][action])
			state[s] = noisy_transition_funcs_[s]->ComputeCurrentIndex(random);
	}
}

OBS_TYPE Parser::GetObservation(const vector<int>& state, int action,
	double& random) const {
	for (int s = 0; s < state.size(); s++) {
		curr_state_vars_[s].curr_value = state[s];
	}
	action_vars_[0].curr_value = action;

	OBS_TYPE obs = 0;
	for (int o = 0; o < obs_vars_.size(); o++) {
		obs = obs * obs_vars_[o].Size()
			+ obs_funcs_[o]->ComputeCurrentIndex(random);
	}
	return obs;
}

double Parser::LogNumStates() const {
	double log_num_states = 0;

	for (int i = 0; i < curr_state_vars_.size(); i++) {
		log_num_states += log2(curr_state_vars_[i].Size());
	}

	return log_num_states;
}

int Parser::NumStates() const {
	if (LogNumStates() > 30) {
		return -1;
	} else {
		int num_states = 1;
		for (int i = 0; i < curr_state_vars_.size(); i++) {
			num_states *= curr_state_vars_[i].Size();
		}

		return num_states;
	}
}

double Parser::LogNumInitialStates() const {
	double log_num_states = 0;

	for (int s = 0; s < curr_state_vars_.size(); s++) {
		double n = 0;
		for (int v = 0; v < curr_state_vars_[s].Size(); v++)
			n += (initial_belief_funcs_[s].GetValue(0, v) > 0);
		log_num_states += log2(n);
	}

	return log_num_states;
}

int Parser::NumInitialStates() const {
	if (LogNumInitialStates() > 30) {
		return -1;
	} else {
		int num_states = 1;

		for (int s = 0; s < prev_state_vars_.size(); s++) {
			int n = 0;
			for (int v = 0; v < prev_state_vars_[s].Size(); v++)
				n += (initial_belief_funcs_[s].GetValue(0, v) > 0);
			num_states *= n;
		}

		return num_states;
	}
}

double Parser::LogNumObservations() const {
	double log_num_obss = 0;

	for (int i = 0; i < obs_vars_.size(); i++) {
		log_num_obss += log2(obs_vars_[i].Size());
	}

	return log_num_obss;
}

OBS_TYPE Parser::NumObservations() const {
	if (LogNumObservations() > 30) {
		return -1;
	} else {
		OBS_TYPE num_obss = 1;

		for (int i = 0; i < obs_vars_.size(); i++) {
			num_obss *= obs_vars_[i].Size();
		}

		return num_obss;
	}
}

double Parser::ObsProb(OBS_TYPE obs, const vector<int>& state,
	int action) const {
	for (int s = 0; s < state.size(); s++) {
		curr_state_vars_[s].curr_value = state[s];
	}
	action_vars_[0].curr_value = action;

	double prob = 1.0;
	for (int o = obs_vars_.size() - 1; o >= 0; o--) {
		prob *= obs_funcs_[o]->GetValue(obs % obs_vars_[o].Size());
		obs /= obs_vars_[o].Size();
	}
	return prob;
}

vector<pair<vector<int>, double> > Parser::ComputeTopTransitions(
	const vector<int>& state, int action, int num) const {
	for (int s = 0; s < state.size(); s++) {
		prev_state_vars_[s].curr_value = state[s];
	}
	action_vars_[0].curr_value = action;

	vector<pair<vector<int>, double> > best;
	best.push_back(pair<vector<int>, double>(vector<int>(), 1.0));
	for (int s = 0; s < state.size(); s++) {
		// Compute sparse next state distribution
		vector<pair<int, double> > distribution;
		int pid = Variable::ComputeCurrentIndex(
			transition_funcs_[s]->parents());
		for (int v = 0; v < prev_state_vars_[s].Size(); v++) {
			double prob = transition_funcs_[s]->GetValue(pid, v);
			if (prob > 0)
				distribution.push_back(pair<int, double>(v, prob));
		}

		// Extend current best state vectors and keep the best
		vector<pair<vector<int>, double> > tmp;
		for (int i = 0; i < best.size(); i++) {
			const pair<vector<int>, double>& it1 = best[i];
			for (int j = 0; j < distribution.size(); j++) {
				const pair<int, double>& it2 = distribution[j];

				// Extend
				double prob = it1.second * it2.second;

				if (tmp.size() == num && prob < tmp.back().second)
					continue;

				vector<int> state = it1.first;
				state.push_back(it2.first);
				pair<vector<int>, double> next(state, prob);

				// Insert in descending order
				int k = 0;
				for (; k < tmp.size(); k++) {
					if (tmp[k].second < next.second)
						break;
				}
				tmp.insert(tmp.begin() + k, next);

				// Keep the best
				if (num != -1 && tmp.size() > num)
					tmp.resize(num);
			}
		}

		best = tmp;
	}
	return best;
}

bool Parser::IsSelfLoopingWithoutReward(const vector<int>& state) const {
	if (selfloop_state_set_.find(state) != selfloop_state_set_.end())
		return true;

	for (int s = 0; s < state.size(); s++) {
		prev_state_vars_[s].curr_value = state[s];
		curr_state_vars_[s].curr_value = state[s];
	}

	for (int a = 0; a < action_vars_[0].Size(); a++) {
		action_vars_[0].curr_value = a;

		for (int s = 0; s < state.size(); s++) {
			if (transition_funcs_[s]->GetValue(state[s]) != 1.0)
				return false;
		}

		for (int r = 0; r < reward_funcs_.size(); r++) {
			if (reward_funcs_[r].GetValue() != 0)
				return false;
		}
	}

	if (enable_selfloop_cache_) {
		if (selfloop_state_set_.size() > 10000) { // Replace the oldest state by the new one
			selfloop_state_set_.erase(selfloop_state_queue_.front());
			selfloop_state_queue_.pop();
		}

		selfloop_state_set_.insert(state);
		selfloop_state_queue_.push(state);
	}

	return true;
}

bool Parser::IsInTerminalStateSet(const vector<int>& state) const {
	for (int i = 0; i < terminal_state_patterns_.size(); i++) {
		const TerminalPattern& pattern = terminal_state_patterns_[i];
		vector<int> val;
		for (int i = 0; i < pattern.state_ids.size(); i++) {
			int id = pattern.state_ids[i];
			val.push_back(state[id]);
		}
		if (pattern.state_vals.find(val) != pattern.state_vals.end()) {
			return true;
		}
	}
	return false;
}

void Parser::PrintState(const vector<int>& state, ostream& out) const {
	out << "[";
	for (int s = 0; s < state.size(); s++) {
		out << (s == 0 ? "" : ", ") << curr_state_vars_[s].name() << ":"
			<< curr_state_vars_[s].GetValue(state[s]);
	}
	out << "]" << endl;
}

void Parser::PrintObs(OBS_TYPE obs, ostream& out) const {
	vector<int> obs_vec(obs_vars_.size());
	for (int o = obs_vars_.size() - 1; o >= 0; o--) {
		obs_vec[o] = obs % obs_vars_[o].Size();
		obs /= obs_vars_[o].Size();
	}

	out << "[";
	for (int o = 0; o < obs_vars_.size(); o++) {
		out << (o == 0 ? "" : ", ") << obs_vars_[o].name() << ":"
			<< obs_vars_[o].GetValue(obs_vec[o]);
	}
	out << "]" << endl;
}

void Parser::PrintAction(int action, ostream& out) const {
	out << action << ":" << action_vars_[0].GetValue(action) << endl;
}

OBS_TYPE Parser::GetPOMDPXObservation(map<string, string>& observe) {
	OBS_TYPE obs = 0;
	for (int o = 0; o < obs_vars_.size(); o++) {
		int index = obs_vars_[o].IndexOf(observe[obs_vars_[o].name()]);
		obs = obs * obs_vars_[o].Size() + index;
	}
	return obs;
}

const string& Parser::GetActionName() {
	return action_vars_[0].name();
}

const string& Parser::GetEnumedAction(int action) {
	return action_vars_[0].GetValue(action);
}

} // namespace despot
