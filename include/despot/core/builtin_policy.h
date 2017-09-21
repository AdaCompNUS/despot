#ifndef BUILTIN_POLICY_H
#define BUILTIN_POLICY_H

#include <vector>
#include <despot/interface/policy.h>

namespace despot {

/* =============================================================================
 * BlindPolicy class
 * =============================================================================*/

class BlindPolicy: public Policy {
private:
	int action_;

public:
	BlindPolicy(const DSPOMDP* model, int action, ParticleLowerBound*
		particle_lower_bound, Belief* belief = NULL);

	int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(int action, OBS_TYPE obs);
};

/* =============================================================================
 * RandomPolicy class
 * =============================================================================*/

class RandomPolicy: public Policy {
private:
	std::vector<double> action_probs_;

public:
	RandomPolicy(const DSPOMDP* model, ParticleLowerBound* ParticleLowerBound,
		Belief* belief = NULL);
	RandomPolicy(const DSPOMDP* model, const std::vector<double>& action_probs,
		ParticleLowerBound* ParticleLowerBound,
		Belief* belief = NULL);

	int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(int action, OBS_TYPE obs);
};

/* =============================================================================
 * ModeStatePolicy class
 * =============================================================================*/

class ModeStatePolicy: public Policy {
private:
	const StateIndexer& indexer_;
	const StatePolicy& policy_;
	mutable std::vector<double> state_probs_;

public:
	ModeStatePolicy(const DSPOMDP* model, const StateIndexer& indexer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);

	int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MMAPStatePolicy class
 * =============================================================================*/

class MMAPStatePolicy: public Policy { // Marginal MAP state policy
private:
	const MMAPInferencer& inferencer_;
	const StatePolicy& policy_;

public:
	MMAPStatePolicy(const DSPOMDP* model, const MMAPInferencer& inferencer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound,
		Belief* belief = NULL);

	int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MajorityActionPolicy class
 * =============================================================================*/

class MajorityActionPolicy: public Policy {
private:
	const StatePolicy& policy_;

public:
	MajorityActionPolicy(const DSPOMDP* model, const StatePolicy& policy,
		ParticleLowerBound* particle_lower_bound, Belief* belief = NULL);

	int Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

} // namespace despot

#endif
