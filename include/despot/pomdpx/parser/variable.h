#ifndef VARIABLE_H
#define VARIABLE_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>

namespace despot {

class NamedVar;
class StateVar;

class Variable {
protected:
  std::vector<std::string> values_; // Possible values of the variable
	std::map<std::string, int> index_;

public:
	int curr_value;

	Variable();
	virtual ~Variable();
	void values(const std::vector<std::string>& values);
	void values(std::string prefix, int num);
	inline const std::vector<std::string>& values() const {
		return values_;
	}
	int IndexOf(std::string val) const;
	const bool HasValue(std::string val) const;
	inline const std::string& GetValue(int v) const {
		return values_[v];
	}
	inline int Size() const {
		return values_.size();
	}

	friend std::ostream& operator<<(std::ostream& os, const Variable& var);

	static std::vector<int> ComputeIndexVec(const std::vector<Variable*>& vars,
		int index);
	static std::vector<int> ComputeIndexVec(const std::vector<NamedVar*>& vars,
		int index);
	static std::vector<int> ComputeIndexVec(const std::vector<StateVar*>& vars,
		int index);
	static int ComputeCurrentIndex(const std::vector<Variable*>& vars);
	static int ComputeCurrentIndex(const std::vector<NamedVar*>& vars);
	static bool IsVariableName(std::string name, const std::vector<NamedVar>& vars);
	static bool IsVariableName(std::string name, const std::vector<StateVar>& vars);
	static bool IsVariableCurrName(std::string name, const std::vector<StateVar>& vars);
	static bool IsVariablePrevName(std::string name, const std::vector<StateVar>& vars);
};

/*---------------------------------------------------------------------------*/

class NamedVar: public Variable {
protected:
	std::string name_;

public:
	NamedVar();
	NamedVar(std::string name);
	virtual ~NamedVar();
	inline void name(std::string str) {
		name_ = str;
	}
	inline const std::string& name() const {
		return name_;
	}

	friend std::ostream& operator<<(std::ostream& os, const NamedVar& var);
};

#define ObsVar NamedVar
#define ActionVar NamedVar
#define RewardVar NamedVar

/*---------------------------------------------------------------------------*/

class StateVar: public NamedVar {
protected:
	std::string prev_name_;
	std::string curr_name_;
	bool observed_;

public:
	StateVar();
	~StateVar();
	inline void prev_name(std::string str) {
		name_ = str;
		prev_name_ = str;
	}
	inline const std::string& prev_name() const {
		return prev_name_;
	}
	inline void curr_name(std::string str) {
		curr_name_ = str;
	}
	inline const std::string& curr_name() const {
		return curr_name_;
	}
	inline void observed(bool o) {
		observed_ = o;
	}
	inline const bool observed() const {
		return observed_;
	}

	friend std::ostream& operator<<(std::ostream& os, const StateVar& var);
};

} // namespace despot

#endif
