#ifndef BUILTIN_POLICY_H
#define BUILTIN_POLICY_H

#include <despot/interface/default_policy.h>
#include <vector>

namespace despot {

/* =============================================================================
 * BlindPolicy class
 * =============================================================================*/

class BlindPolicy: public DefaultPolicy {
private:
	ACT_TYPE action_;

public:
	BlindPolicy(const DSPOMDP* model, ACT_TYPE action, ParticleLowerBound*
		particle_lower_bound);

	ACT_TYPE Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(ACT_TYPE action, OBS_TYPE obs);
};

/* =============================================================================
 * RandomPolicy class
 * =============================================================================*/

class RandomPolicy: public DefaultPolicy {
private:
	std::vector<double> action_probs_;

public:
	RandomPolicy(const DSPOMDP* model, ParticleLowerBound* ParticleLowerBound);
	RandomPolicy(const DSPOMDP* model, const std::vector<double>& action_probs,
		ParticleLowerBound* ParticleLowerBound);

	ACT_TYPE Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;

	ValuedAction Search();
	void Update(ACT_TYPE action, OBS_TYPE obs);
};

/* =============================================================================
 * ModeStatePolicy class
 * =============================================================================*/

class ModeStatePolicy: public DefaultPolicy {
private:
	const StateIndexer& indexer_;
	const StatePolicy& policy_;
	mutable std::vector<double> state_probs_;

public:
	ModeStatePolicy(const DSPOMDP* model, const StateIndexer& indexer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound);

	ACT_TYPE Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MMAPStatePolicy class
 * =============================================================================*/

class MMAPStatePolicy: public DefaultPolicy { // Marginal MAP state policy
private:
	const MMAPInferencer& inferencer_;
	const StatePolicy& policy_;

public:
	MMAPStatePolicy(const DSPOMDP* model, const MMAPInferencer& inferencer,
		const StatePolicy& policy, ParticleLowerBound* particle_lower_bound);

	ACT_TYPE Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

/* =============================================================================
 * MajorityActionPolicy class
 * =============================================================================*/

class MajorityActionPolicy: public DefaultPolicy {
private:
	const StatePolicy& policy_;

public:
	MajorityActionPolicy(const DSPOMDP* model, const StatePolicy& policy,
		ParticleLowerBound* particle_lower_bound);

	ACT_TYPE Action(const std::vector<State*>& particles, RandomStreams& streams,
		History& history) const;
};

} // namespace despot

#endif
