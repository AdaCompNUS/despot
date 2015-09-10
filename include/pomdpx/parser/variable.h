#ifndef VARIABLE_H
#define VARIABLE_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>

using namespace std;

class NamedVar;
class StateVar;

class Variable {
protected:
	vector<string> values_; // Possible values of the variable
	map<string, int> index_;

public:
	int curr_value;

	Variable();
	virtual ~Variable();
	void values(const vector<string>& values);
	void values(string prefix, int num);
	inline const vector<string>& values() const {
		return values_;
	}
	int IndexOf(string val) const;
	const bool HasValue(string val) const;
	inline const string& GetValue(int v) const {
		return values_[v];
	}
	inline int Size() const {
		return values_.size();
	}

	friend ostream& operator<<(ostream& os, const Variable& var);

	static vector<int> ComputeIndexVec(const vector<Variable*>& vars,
		int index);
	static vector<int> ComputeIndexVec(const vector<NamedVar*>& vars,
		int index);
	static vector<int> ComputeIndexVec(const vector<StateVar*>& vars,
		int index);
	static int ComputeCurrentIndex(const vector<Variable*>& vars);
	static int ComputeCurrentIndex(const vector<NamedVar*>& vars);
	static bool IsVariableName(string name, const vector<NamedVar>& vars);
	static bool IsVariableName(string name, const vector<StateVar>& vars);
	static bool IsVariableCurrName(string name, const vector<StateVar>& vars);
	static bool IsVariablePrevName(string name, const vector<StateVar>& vars);
};

/*---------------------------------------------------------------------------*/

class NamedVar: public Variable {
protected:
	string name_;

public:
	NamedVar();
	NamedVar(string name);
	virtual ~NamedVar();
	inline void name(string str) {
		name_ = str;
	}
	inline const string& name() const {
		return name_;
	}

	friend ostream& operator<<(ostream& os, const NamedVar& var);
};

#define ObsVar NamedVar
#define ActionVar NamedVar
#define RewardVar NamedVar

/*---------------------------------------------------------------------------*/

class StateVar: public NamedVar {
protected:
	string prev_name_;
	string curr_name_;
	bool observed_;

public:
	StateVar();
	~StateVar();
	inline void prev_name(string str) {
		name_ = str;
		prev_name_ = str;
	}
	inline const string& prev_name() const {
		return prev_name_;
	}
	inline void curr_name(string str) {
		curr_name_ = str;
	}
	inline const string& curr_name() const {
		return curr_name_;
	}
	inline void observed(bool o) {
		observed_ = o;
	}
	inline const bool observed() const {
		return observed_;
	}

	friend ostream& operator<<(ostream& os, const StateVar& var);
};

#endif
