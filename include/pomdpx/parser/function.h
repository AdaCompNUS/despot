#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <vector>
#include <iterator>
#include <map>
#include <cassert>
#include <cmath>
#include "core/globals.h"
#include "variable.h"
#include "util/util.h"

using namespace std;

/* =============================================================================
 * Function class
 * =============================================================================*/

/**
 * Implementation for double-valued multivariate functions. For the convenience
 * of implementing conditional probability tables, one of the variables is
 * designated as the child variable, and the remaining ones the parent
 * variables.
 */

class Function {
	friend class CPT;

protected:
	NamedVar* child_;
	vector<NamedVar*> parents_;

	vector<vector<double> > values_; // values_[parents][child]
	vector<map<int, double> > map_; // map_[parents][child]

	bool SetValue(vector<string>& keys, int key_pos, int pid, int cid,
		const vector<double>& values, int start, int end);

public:
	Function();
	Function(NamedVar* child, vector<NamedVar*> parents);
	virtual ~Function();

	virtual bool SetValue(const vector<string>& keys,
		const vector<double>& values);
	virtual void SetValue(int pid, int cid, double value);
	virtual double GetValue(int pid, int cid) const;
	virtual double GetValue(int cid = 0) const;

	virtual const vector<NamedVar*>& parents() const;
	virtual int ParentSize() const;
	virtual const NamedVar* child() const;
	virtual int ChildSize() const;

	virtual double ComputeConstrainedMaximum(const NamedVar* var,
		int value) const;
	virtual double ComputeConstrainedMinimum(const NamedVar* var,
		int value) const;

	friend ostream& operator<<(std::ostream& os, const Function& func);
};

/* =============================================================================
 * CPT class
 * =============================================================================*/

class CPT: public Function { // Conditional Probability Table
public:
	virtual ~CPT();

	virtual bool Validate() const = 0;
	virtual int ComputeIndex(int pid, double& sum) const = 0;
	virtual int ComputeCurrentIndex(double& sum) const = 0;

	virtual void ComputeSparseChildDistribution() = 0;
	virtual bool IsIdentityUnderConstraint(const NamedVar* var,
		int value) const = 0;
	virtual CPT* CreateNoisyVariant(double noise) const = 0;
};

/* =============================================================================
 * TabularCPT class
 * =============================================================================*/

class TabularCPT: public CPT {
	friend class HierarchyCPT;

protected:
	vector<vector<pair<int, double> > > sparse_values_;

public:
	TabularCPT(NamedVar* child, vector<NamedVar*> parents);

	bool Validate() const;
	int ComputeIndex(int pid, double& sum) const;
	int ComputeCurrentIndex(double& sum) const;

	void ComputeSparseChildDistribution();
	bool IsIdentityUnderConstraint(const NamedVar* var, int value) const;
	CPT* CreateNoisyVariant(double noise) const;
};

/* =============================================================================
 * HierarchyCPT class
 * =============================================================================*/

/**
 * This sparse reprentation assumes that if the value of the first parent is
 * known, then the set of parents that the child depends on will be smaller.
 */
class HierarchyCPT: public CPT {
protected:
	vector<TabularCPT*> cpts_;

public:
	HierarchyCPT(NamedVar* child, vector<NamedVar*> parents);
	virtual ~HierarchyCPT();

	void SetParents(int val, vector<NamedVar*> parents);

	virtual bool SetValue(int val, const vector<string>& keys,
		const vector<double>& values);
	inline bool SetValue(const vector<string>& keys,
		const vector<double>& values) {
		assert(false);
		return false;
	}
	;
	inline void SetValue(int pid, int cid, double value) {
		assert(false);
	}
	inline virtual double GetValue(int pid, int cid) const {
		assert(false);
		return 0.0;
	}
	virtual double GetValue(int cid = 0) const;

	virtual int ComputeIndex(int pid, double& sum) const {
		assert(false);
		return 0;
	}
	virtual int ComputeCurrentIndex(double& sum) const;

	virtual int ParentSize() const;

	virtual bool Validate() const;

	virtual void ComputeSparseChildDistribution();
	virtual bool IsIdentityUnderConstraint(const NamedVar* var,
		int value) const;
	virtual CPT* CreateNoisyVariant(double noise) const;

	inline virtual double ComputeConstrainedMaximum(const NamedVar* var,
		int value) const {
		assert(false);
		return 0.0;
	}
	;
	inline virtual double ComputeConstrainedMinimum(const NamedVar* var,
		int value) const {
		assert(false);
		return 0.0;
	}
	;

	friend ostream& operator<<(std::ostream& os, const HierarchyCPT& cpt);
};

#endif
