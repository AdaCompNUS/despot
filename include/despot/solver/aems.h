#ifndef AEMS_H
#define AEMS_H

#include <despot/core/solver.h>
#include <despot/core/pomdp.h>
#include <despot/core/belief.h>
#include <despot/core/node.h>
#include <despot/core/globals.h>

namespace despot {

class AEMS: public Solver {
private:
	VNode* root_;
	BeliefLowerBound* lower_bound_;
	BeliefUpperBound* upper_bound_;
	SearchStatistics statistics_;
	const BeliefMDP* model_;
	bool reuse_;

public:
	AEMS(const DSPOMDP* model, BeliefLowerBound* lower_bound,
		BeliefUpperBound* upper_bound, Belief* belief = NULL);

	ValuedAction Search();
	virtual void Update(int action, OBS_TYPE obs);
	virtual void belief(Belief* belief);

private:
	static void InitLowerBound(VNode* vnode, BeliefLowerBound* lower_bound,
		History& history);
	static void InitUpperBound(VNode* vnode, BeliefUpperBound* upper_bound,
		History& history);

	static void Expand(QNode* qnode, BeliefLowerBound* lower_bound,
		BeliefUpperBound* upper_bound, const BeliefMDP* model, History& history);
	static void Expand(VNode* vnode, BeliefLowerBound* lower_bound,
		BeliefUpperBound* upper_bound, const BeliefMDP* model, History& history);
	static void Backup(VNode* vnode);
	static void Update(VNode* vnode);
	static void Update(QNode* qnode);
	static VNode* FindMaxApproxErrorLeaf(VNode* root);
	static void FindMaxApproxErrorLeaf(VNode* vnode, double likelihood,
		double& bestAE, VNode*& bestNode);
	static void FindMaxApproxErrorLeaf(QNode* qnode, double likelihood,
		double& bestAE, VNode*& bestNode);

	static ValuedAction OptimalAction(const VNode* vnode);
	static double Likelihood(QNode* qnode);
	static double AEMS2Likelihood(QNode* qnode);
};

} // namespace despot

#endif
