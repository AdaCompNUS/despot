#ifndef NODE_H
#define NODE_H

#include <despot/core/pomdp.h>
#include <despot/util/util.h>
#include <despot/random_streams.h>
#include <despot/util/logging.h>

namespace despot {

class QNode;

/* =============================================================================
 * VNode class
 * =============================================================================*/

/**
 * A belief/value/AND node in the search tree.
 */
class VNode {
protected:
  std::vector<State*> particles_; // Used in DESPOT
	Belief* belief_; // Used in AEMS
	int depth_;
	QNode* parent_;
	OBS_TYPE edge_;

	std::vector<QNode*> children_;

	ValuedAction default_move_; // Value and action given by default policy
	double lower_bound_;
	double upper_bound_;

	// For POMCP
	int count_; // Number of visits on the node
	double value_; // Value of the node

public:
	VNode* vstar;
	double likelihood; // Used in AEMS
	double utility_upper_bound;

	VNode(std::vector<State*>& particles, int depth = 0, QNode* parent = NULL,
		OBS_TYPE edge = -1);
	VNode(Belief* belief, int depth = 0, QNode* parent = NULL, OBS_TYPE edge =
		-1);
	VNode(int count, double value, int depth = 0, QNode* parent = NULL,
		OBS_TYPE edge = -1);
	~VNode();

	Belief* belief() const;
	const std::vector<State*>& particles() const;
	void depth(int d);
	int depth() const;
	void parent(QNode* parent);
	QNode* parent();
	OBS_TYPE edge();

	double Weight() const;

	const std::vector<QNode*>& children() const;
	std::vector<QNode*>& children();
	const QNode* Child(int action) const;
	QNode* Child(int action);
	int Size() const;
	int PolicyTreeSize() const;

	void default_move(ValuedAction move);
	ValuedAction default_move() const;
	void lower_bound(double value);
	double lower_bound() const;
	void upper_bound(double value);
	double upper_bound() const;

	bool IsLeaf();

	void Add(double val);
	void count(int c);
	int count() const;
	void value(double v);
	double value() const;

	void PrintTree(int depth = -1, std::ostream& os = std::cout);
	void PrintPolicyTree(int depth = -1, std::ostream& os = std::cout);

	void Free(const DSPOMDP& model);
};

/* =============================================================================
 * QNode class
 * =============================================================================*/

/**
 * A Q-node/AND-node (child of a belief node) of the search tree.
 */
class QNode {
protected:
	VNode* parent_;
	int edge_;
	std::map<OBS_TYPE, VNode*> children_;
	double lower_bound_;
	double upper_bound_;

	// For POMCP
	int count_; // Number of visits on the node
	double value_; // Value of the node

public:
	double default_value;
	double utility_upper_bound;
	double step_reward;
	double likelihood;
	VNode* vstar;

	QNode(VNode* parent, int edge);
	QNode(int count, double value);
	~QNode();

	void parent(VNode* parent);
	VNode* parent();
	int edge();
	std::map<OBS_TYPE, VNode*>& children();
	VNode* Child(OBS_TYPE obs);
	int Size() const;
	int PolicyTreeSize() const;

	double Weight() const;

	void lower_bound(double value);
	double lower_bound() const;
	void upper_bound(double value);
	double upper_bound() const;

	void Add(double val);
	void count(int c);
	int count() const;
	void value(double v);
	double value() const;
};

} // namespace despot

#endif
