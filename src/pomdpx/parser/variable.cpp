#include <despot/pomdpx/parser/variable.h>
#include <iostream>
#include <string>
#include <despot/util/util.h>

using namespace std;

namespace despot {

Variable::Variable() {
}

Variable::~Variable() {
}

void Variable::values(const vector<string>& ve) {
	values_ = ve;
	for (int i = 0; i < values_.size(); i++)
		index_[values_[i]] = i;
}

void Variable::values(string prefix, int num) {
	values_.resize(num);
	for (int i = 0; i < num; i++)
		values_[i] = prefix + to_string(i);

	// NOTE: Index can be computed by stripping of the prefix
	for (int i = 0; i < values_.size(); i++)
		index_[values_[i]] = i;
}

ostream& operator<<(ostream& os, const Variable& var) {
	os << "Values:";
	for (unsigned int i = 0; i < var.values_.size(); i++) {
		os << " " << i << "=" << var.values_[i];
	}
	os << endl;
	return os;
}

int Variable::IndexOf(string value) const {
	map<string, int>::const_iterator it = index_.find(value);
	return (it != index_.end()) ? it->second : -1;
}

const bool Variable::HasValue(string value) const {
	for (int i = 0; i < values_.size(); i++)
		if (values_[i] == value)
			return true;
	return false;
}

vector<int> Variable::ComputeIndexVec(const vector<Variable*>& vars,
	int index) {
	vector<int> vec(vars.size());
	for (int i = vec.size() - 1; i >= 0; i--) {
		vec[i] = index % vars[i]->Size();
		index /= vars[i]->Size();
	}

	return vec;
}

vector<int> Variable::ComputeIndexVec(const vector<NamedVar*>& vars,
	int index) {
	vector<int> vec(vars.size());
	for (int i = vec.size() - 1; i >= 0; i--) {
		vec[i] = index % vars[i]->Size();
		index /= vars[i]->Size();
	}

	return vec;
}

vector<int> Variable::ComputeIndexVec(const vector<StateVar*>& vars,
	int index) {
	vector<int> vec(vars.size());
	for (int i = vec.size() - 1; i >= 0; i--) {
		vec[i] = index % vars[i]->Size();
		index /= vars[i]->Size();
	}

	return vec;
}

int Variable::ComputeCurrentIndex(const vector<Variable*>& vars) {
	int index = 0;
	for (int i = 0; i < vars.size(); i++) {
		Variable* var = vars[i];
		index = index * var->Size() + var->curr_value;
	}
	return index;
}

int Variable::ComputeCurrentIndex(const vector<NamedVar*>& vars) {
	int index = 0;
	for (int i = 0; i < vars.size(); i++) {
		NamedVar* var = vars[i];
		index = index * var->Size() + var->curr_value;
	}
	return index;
}

bool Variable::IsVariableName(string name, const vector<NamedVar>& vars) {
	for (int i = 0; i < vars.size(); i++) {
		const NamedVar& var = vars[i];
		if (name == var.name())
			return true;
	}
	return false;
}

bool Variable::IsVariableName(string name, const vector<StateVar>& vars) {
	for (int i = 0; i < vars.size(); i++) {
		const StateVar& var = vars[i];
		if (name == var.name())
			return true;
	}
	return false;
}

bool Variable::IsVariableCurrName(string name, const vector<StateVar>& vars) {
	for (int i = 0; i < vars.size(); i++) {
		const StateVar& var = vars[i];
		if (name == var.curr_name())
			return true;
	}
	return false;
}

bool Variable::IsVariablePrevName(string name, const vector<StateVar>& vars) {
	for (int i = 0; i < vars.size(); i++) {
		const StateVar& var = vars[i];
		if (name == var.prev_name())
			return true;
	}
	return false;
}

/*---------------------------------------------------------------------------*/

NamedVar::NamedVar() {
}

NamedVar::NamedVar(string name) :
	name_(name) {
}

NamedVar::~NamedVar() {
}

ostream& operator<<(std::ostream& os, const NamedVar& var) {
	os << "Name:" << var.name_ << endl;
	os << "Values:";

	for (int i = 0; i < var.values_.size(); i++) {
		os << " " << i << "=" << var.values_[i];
	}
	return os;
}

/*---------------------------------------------------------------------------*/

StateVar::StateVar() {
}

StateVar::~StateVar() {
}

ostream& operator<<(ostream& os, const StateVar& var) {
	os << "Name: " << var.name_ << endl;
	os << "PrevName:" << var.prev_name_ << endl;
	os << "CurrName:" << var.curr_name_ << endl;
	os << "FullyObs:" << var.observed_ << endl;
	os << "Values:";

	for (unsigned int i = 0; i < var.values_.size(); i++) {
		os << " " << i << "=" << var.values_[i];
	}
	return os;
}

} // namespace despot
