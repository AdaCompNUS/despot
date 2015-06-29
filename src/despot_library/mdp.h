#ifndef MDP_H
#define MDP_H

#include "pomdp.h"

/**
 * Interface for a discrete MDP. This class implements the following functions:
 * <ol>
 * <li> value iteration,
 * <li> computation of alpha vectors and POMDP value for fixed-action policies.
 * </ol>
 */
class MDP {
protected:
	vector<ValuedAction> policy_;

	vector<vector<double> > blind_alpha_; // For blind policy

public:
	virtual ~MDP();

	virtual int NumStates() const = 0;
	virtual int NumActions() const = 0;
	virtual const vector<State>& TransitionProbability(int s, int a) const = 0;
	virtual double Reward(int s, int a) const = 0;

	virtual void ComputeOptimalPolicyUsingVI();
	const vector<ValuedAction>& policy() const;

	virtual void ComputeBlindAlpha();
	double ComputeActionValue(const ParticleBelief* belief,
		const StateIndexer& indexer, int action) const;
	const vector<vector<double> >& blind_alpha() const;
};

#endif
