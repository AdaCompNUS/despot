#include <despot/interface/upper_bound.h>
#include <despot/interface/pomdp.h>
#include <despot/core/mdp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * ScenarioUpperBound
 * =============================================================================*/

ScenarioUpperBound::ScenarioUpperBound() {
}

ScenarioUpperBound::~ScenarioUpperBound() {
}

void ScenarioUpperBound::Init(const RandomStreams& streams) {
}

/* =============================================================================
 * ParticleUpperBound
 * =============================================================================*/

ParticleUpperBound::ParticleUpperBound() {
}

ParticleUpperBound::~ParticleUpperBound() {
}

double ParticleUpperBound::Value(const vector<State*>& particles,
	RandomStreams& streams, History& history) const {
	double value = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		value += particle->weight * Value(*particle);
	}
	return value;
}

/* =============================================================================
 * BeliefUpperBound
 * =============================================================================*/

BeliefUpperBound::BeliefUpperBound() {
}

BeliefUpperBound::~BeliefUpperBound() {
}

} // namespace despot
