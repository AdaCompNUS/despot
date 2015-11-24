#include <queue>

#include "tag.h"
#include <despot/util/coord.h>
#include <despot/util/floor.h>

using namespace std;

namespace despot {

/* =============================================================================
 * Tag class
 * =============================================================================*/

Tag::Tag() {
	string map = string("mapSize = 5 10\n") + string("#####...##\n")
		+ string("#####...##\n") + string("#####...##\n")
		+ string("...........\n") + string("...........");
	istringstream iss(map);
	Init(iss);

	same_loc_obs_ = floor_.NumCells();
	obs_.resize(NumStates());
	for (int rob = 0; rob < floor_.NumCells(); rob++) {
		for (int opp = 0; opp < floor_.NumCells(); opp++) {
			int s = RobOppIndicesToStateIndex(rob, opp);
			obs_[s] = (rob == opp ? same_loc_obs_ : rob);
		}
	}
  robot_pos_unknown_ = false;
}

Tag::Tag(string params_file) :
	BaseTag(params_file) {
	same_loc_obs_ = floor_.NumCells();
	obs_.resize(NumStates());
	for (int rob = 0; rob < floor_.NumCells(); rob++) {
		for (int opp = 0; opp < floor_.NumCells(); opp++) {
			int s = RobOppIndicesToStateIndex(rob, opp);
			obs_[s] = (rob == opp ? same_loc_obs_ : rob);
		}
	}
  robot_pos_unknown_ = false;
}

bool Tag::Step(State& state, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	bool terminal = BaseTag::Step(state, random_num, action, reward);

	obs = obs_[state.state_id];

	return terminal;
}

double Tag::ObsProb(OBS_TYPE obs, const State& s, int a) const {
	const TagState& state = static_cast<const TagState&>(s);

	return obs == obs_[state.state_id];
}

Belief* Tag::ExactPrior() const {
	vector<State*> particles;
	for (int rob = 0; rob < floor_.NumCells(); rob++) {
		for (int opp = 0; opp < floor_.NumCells(); opp++) {
			TagState* state = static_cast<TagState*>(BaseTag::Allocate(
				RobOppIndicesToStateIndex(rob, opp),
				1.0 / floor_.NumCells() / floor_.NumCells()));
			particles.push_back(state);
		}
	}

	TagBelief* belief = new TagBelief(particles, this);
	belief->state_indexer(this);
	return belief;
}

Belief* Tag::ApproxPrior() const {
	vector<State*> particles;

	int N = floor_.NumCells();
	double wgt = 1.0 / N / N;
	for (int rob = 0; rob < N; rob++) {
		for (int opp = 0; opp < N; opp++) {
			TagState* state = static_cast<TagState*>(BaseTag::Allocate(
				RobOppIndicesToStateIndex(rob, opp), wgt));
			particles.push_back(state);
		}
	}

	ParticleBelief* belief = new ParticleBelief(particles, this);
	belief->state_indexer(this);
	return belief;
}

Belief* Tag::InitialBelief(const State* start, string type) const {
	Belief* prior = NULL;
	if (type == "EXACT") {
		prior = ExactPrior();
	} else if (type == "DEFAULT" || type == "PARTICLE") {
		prior = ApproxPrior();
	} else {
		cerr << "[Tag::InitialBelief] Unsupported belief type: " << type << endl;
		exit(0);
	}

	return prior;
}

void Tag::Observe(const Belief* belief, int action,
	map<OBS_TYPE, double>& obss) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();
	for (int i = 0; i < particles.size(); i++) {
		TagState* state = static_cast<TagState*>(particles[i]);
		const vector<State>& distribution = transition_probabilities_[GetIndex(
			state)][action];
		for (int i = 0; i < distribution.size(); i++) {
			const State& next = distribution[i];
			OBS_TYPE obs = obs_[next.state_id];
			double p = state->weight * next.weight;
			obss[obs] += p;
		}
	}
}

void Tag::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	if (obs == floor_.NumCells()) {
		out << "On opponent" << endl;
	} else {
		Coord rob = floor_.GetCell(obs);
		out << "Rob at (" << rob.x << ", " << rob.y << ")" << endl;
	}
}

} // namespace despot
