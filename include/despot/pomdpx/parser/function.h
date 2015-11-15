#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <vector>
#include <iterator>
#include <map>
#include <cassert>
#include <cmath>
#include <despot/core/globals.h>
#include <despot/pomdpx/parser/variable.h>
#include <despot/util/util.h>

namespace despot {

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
  std::vector<NamedVar*> parents_;

  std::vector<std::vector<double> > values_; // values_[parents][child]
  std::vector<std::map<int, double> > map_; // map_[parents][child]

	bool SetValue(std::vector<std::string>& keys, int key_pos, int pid, int cid,
		const std::vector<double>& values, int start, int end);

public:
	Function();
	Function(NamedVar* child, std::vector<NamedVar*> parents);
	virtual ~Function();

	virtual bool SetValue(const std::vector<std::string>& keys,
		const std::vector<double>& values);
	virtual void SetValue(int pid, int cid, double value);
	virtual double GetValue(int pid, int cid) const;
	virtual double GetValue(int cid = 0) const;

	virtual const std::vector<NamedVar*>& parents() const;
	virtual int ParentSize() const;
	virtual const NamedVar* child() const;
	virtual int ChildSize() const;

	virtual double ComputeConstrainedMaximum(const NamedVar* var,
		int value) const;
	virtual double ComputeConstrainedMinimum(const NamedVar* var,
		int value) const;

	friend std::ostream& operator<<(std::ostream& os, const Function& func);
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
  std::vector<std::vector<std::pair<int, double> > > sparse_values_;

public:
	TabularCPT(NamedVar* child, std::vector<NamedVar*> parents);

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
  std::vector<TabularCPT*> cpts_;

public:
	HierarchyCPT(NamedVar* child, std::vector<NamedVar*> parents);
	virtual ~HierarchyCPT();

	void SetParents(int val, std::vector<NamedVar*> parents);

	virtual bool SetValue(int val, const std::vector<std::string>& keys,
		const std::vector<double>& values);
	inline bool SetValue(const std::vector<std::string>& keys,
		const std::vector<double>& values) {
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

	friend std::ostream& operator<<(std::ostream& os, const HierarchyCPT& cpt);
};

} // namespace despot

#endif
