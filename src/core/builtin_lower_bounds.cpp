#include <despot/core/builtin_lower_bounds.h>
#include <despot/interface/pomdp.h>
#include <despot/core/node.h>
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * POMCPScenarioLowerBound class
 * =============================================================================*/

POMCPScenarioLowerBound::POMCPScenarioLowerBound(const DSPOMDP* model,
	POMCPPrior* prior,
	Belief* belief) :
	ScenarioLowerBound(model/*, belief*/),
	prior_(prior) {
	explore_constant_ = model_->GetMaxReward()
		- model_->GetBestAction().value;
}

ValuedAction POMCPScenarioLowerBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	prior_->history(history);
	VNode* root = POMCP::CreateVNode(0, particles[0], prior_, model_);
	// Note that particles are assumed to be of equal weight
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		State* copy = model_->Copy(particle);
		POMCP::Simulate(copy, streams, root, model_, prior_);
		model_->Free(copy);
	}

	ValuedAction va = POMCP::OptimalAction(root);
	va.value *= State::Weight(particles);
	delete root;
	return va;
}

/* =============================================================================
 * TrivialParticleLowerBound class
 * =============================================================================*/

TrivialParticleLowerBound::TrivialParticleLowerBound(const DSPOMDP* model) :
	ParticleLowerBound(model) {
}

ValuedAction TrivialParticleLowerBound::Value(
	const vector<State*>& particles) const {
	ValuedAction va = model_->GetBestAction();
	va.value *= State::Weight(particles) / (1 - Globals::Discount());
	return va;
}

/* =============================================================================
 * TrivialBeliefLowerBound class
 * =============================================================================*/

TrivialBeliefLowerBound::TrivialBeliefLowerBound(const DSPOMDP* model) :
	BeliefLowerBound(model) {
}

ValuedAction TrivialBeliefLowerBound::Value(const Belief* belief) const {
	ValuedAction va = model_->GetBestAction();
	va.value *= 1.0 / (1 - Globals::Discount());
	return va;
}

} // namespace despot
