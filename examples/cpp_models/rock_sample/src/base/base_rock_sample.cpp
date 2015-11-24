#include "base_rock_sample.h"

using namespace std;

namespace despot {

/* ==============================================================================
 * RockSampleState class
 * ==============================================================================*/

RockSampleState::RockSampleState() {
}

RockSampleState::RockSampleState(int _state_id) {
	state_id = _state_id;
}

string RockSampleState::text() const {
	return "id = " + to_string(state_id);
}

/* ==============================================================================
 * RockSample class
 * ==============================================================================*/

BaseRockSample::BaseRockSample(string map) {
	ifstream fin(map.c_str(), ifstream::in);
	string tmp;
	fin >> tmp >> tmp >> size_ >> size_;

	char tok;
	for (int r = size_ - 1; r >= 0; r--) {
		for (int c = 0; c < size_; c++) {
			fin >> tok;
			if (tok == 'R')
				start_pos_ = Coord(c, r);
			else if (tok == '-') {
				rock_pos_.push_back(Coord(c, r));
			}
		}
	}
	num_rocks_ = rock_pos_.size();

	grid_.Resize(size_, size_);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rock_pos_[i]) = i;
	}

	InitStates();

	/*
	 clock_t start = clock();
	 cerr << "Initializing transitions" << endl;
	 InitializeTransitions();
	 cerr << "Done " << (double (clock() - start) / CLOCKS_PER_SEC) << endl;
	 ComputeOptimalPolicyUsingVI();
	 */
}

BaseRockSample::BaseRockSample(int size, int rocks) :
	grid_(size, size),
	size_(size),
	num_rocks_(rocks) {
	if (size == 4 && rocks == 4) {
		Init_4_4();
	} else if (size == 5 && rocks == 5) {
		Init_5_5();
	} else if (size == 5 && rocks == 7) {
		Init_5_7();
	} else if (size == 7 && rocks == 8) {
		Init_7_8();
	} else if (size == 11 && rocks == 11) {
		Init_11_11();
	} else {
		InitGeneral();
	}

	// InitStates();
}

void BaseRockSample::InitStates() {
	states_.resize(NumStates());

	for (int s = 0; s < NumStates(); s++) {
		states_[s] = new RockSampleState(s);
	}
}

void BaseRockSample::InitGeneral() {
	start_pos_ = Coord(0, size_ / 2);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; i++) {
		Coord pos;
		do {
			pos = Coord(Random::RANDOM.NextInt(size_),
				Random::RANDOM.NextInt(size_));
		} while (grid_(pos) >= 0);
		grid_(pos) = i;
		rock_pos_.push_back(pos);
	}
}

void BaseRockSample::Init_4_4() {
	cout << "Using special layout for rocksample(4, 4)" << endl;

	Coord rocks[] = { Coord(3, 1), Coord(2, 1), Coord(1, 3), Coord(1, 0) };

	start_pos_ = Coord(0, 2);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rocks[i]) = i;
		rock_pos_.push_back(rocks[i]);
	}
}

void BaseRockSample::Init_5_5() {
	cout << "Using special layout for rocksample(5, 5)" << endl;

	Coord rocks[] = { Coord(2, 4), Coord(0, 4), Coord(3, 3), Coord(2, 2), Coord(
		4, 1) };

	start_pos_ = Coord(0, 2);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rocks[i]) = i;
		rock_pos_.push_back(rocks[i]);
	}
}

void BaseRockSample::Init_5_7() {
	cout << "Using special layout for rocksample(5, 7)" << endl;

	Coord rocks[] = { Coord(1, 0), Coord(2, 1), Coord(1, 2), Coord(2, 2), Coord(
		4, 2), Coord(0, 3), Coord(3, 4) };

	start_pos_ = Coord(0, 2);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rocks[i]) = i;
		rock_pos_.push_back(rocks[i]);
	}
}

void BaseRockSample::Init_7_8() {
	// Equivalent to RockSample_7_8.pomdpx
	cout << "Using special layout for rocksample(7, 8)" << endl;

	Coord rocks[] = { Coord(2, 0), Coord(0, 1), Coord(3, 1), Coord(6, 3), Coord(
		2, 4), Coord(3, 4), Coord(5, 5), Coord(1, 6) };

	start_pos_ = Coord(0, 3);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rocks[i]) = i;
		rock_pos_.push_back(rocks[i]);
	}
}

void BaseRockSample::Init_11_11() {
	// Equivalent to RockSample_11_11.pomdp(x)
	cout << "Using special layout for rocksample(11, 11)" << endl;

	Coord rocks[] = { Coord(0, 3), Coord(0, 7), Coord(1, 8), Coord(2, 4), Coord(
		3, 3), Coord(3, 8), Coord(4, 3), Coord(5, 8), Coord(6, 1), Coord(9, 3),
		Coord(9, 9) };

	start_pos_ = Coord(0, 5);
	grid_.SetAllValues(-1);
	for (int i = 0; i < num_rocks_; ++i) {
		grid_(rocks[i]) = i;
		rock_pos_.push_back(rocks[i]);
	}
}

State* BaseRockSample::CreateStartState(string type) const {
	int state = (1 << num_rocks_) * CoordToIndex(start_pos_);
	for (int i = 0; i < num_rocks_; i++) {
		if (Random::RANDOM.NextInt(2))
			SetFlag(state, i);
	}
	return new RockSampleState(state);
}

class RockSampleBelief: public ParticleBelief {
private:
	const BaseRockSample* rs_model_;
public:
	RockSampleBelief(vector<State*> particles, const DSPOMDP* model,
		Belief* prior = NULL) :
		ParticleBelief(particles, model, prior, false),
		rs_model_(static_cast<const BaseRockSample*>(model)) {
	}

	Belief* MakeCopy() const {
		vector<State*> copy;
		for (int i = 0; i < particles_.size(); i++) {
			State* particle = particles_[i];
			copy.push_back(model_->Copy(particle));
		}

		return new RockSampleBelief(copy, model_, prior_);
	}

	void Update(int action, OBS_TYPE obs) { // TODO: Not complete yet
		Belief* updated = rs_model_->Tau(this, action, obs);

		for (int i = 0; i < particles_.size(); i++)
			rs_model_->Free(particles_[i]);
		particles_.clear();

		const vector<State*>& new_particles =
			static_cast<ParticleBelief*>(updated)->particles();
		for (int i = 0; i < new_particles.size(); i++)
			particles_.push_back(rs_model_->Copy(new_particles[i]));

		delete updated;
	}
};

vector<State*> BaseRockSample::InitialParticleSet() const {
	vector<State*> particles;
	int N = 1 << num_rocks_, pos = CoordToIndex(start_pos_) * (1 << num_rocks_);
	for (int n = 0; n < N; n++) {
		RockSampleState* rockstate = static_cast<RockSampleState*>(Allocate(
			n + pos, 1.0 / N));
		particles.push_back(rockstate);
	}
	return particles;
}

/**
 * Initial position of the robot is uniformly distributed on the west side.
 */
vector<State*> BaseRockSample::NoisyInitialParticleSet() const {
	vector<State*> particles;
	int N = 1 << num_rocks_;
	for (int y = 1; y < size_ - 1; y++) {
		int pos = CoordToIndex(Coord(0, y)) * (1 << num_rocks_);
		for (int n = 0; n < N; n++) {
			RockSampleState* rockstate = static_cast<RockSampleState*>(Allocate(
				n + pos, 1.0 / N / (size_ - 2)));
			particles.push_back(rockstate);
		}
	}
	return particles;
}

Belief* BaseRockSample::InitialBelief(const State* start, string type) const {
	if (type == "DEFAULT" || type == "PARTICLE") {
		return new ParticleBelief(InitialParticleSet(), this);
	} else if (type == "NOISY") {
		return new ParticleBelief(NoisyInitialParticleSet(), this);
	} else {
		cerr << "Unsupported initial belief type: " << type << endl;
		exit(0);
	}
}

class RockSampleParticleUpperBound1: public ParticleUpperBound {
protected:
	const BaseRockSample* rs_model_;
public:
	RockSampleParticleUpperBound1(const BaseRockSample* model) :
		rs_model_(model) {
	}

	double Value(const State& state) const {
		const RockSampleState& rockstate =
			static_cast<const RockSampleState&>(state);
		int count = 0;
		for (int rock = 0; rock < rs_model_->num_rocks_; rock++)
			count += rs_model_->GetRock(&rockstate, rock);
		return 10.0 * (1 - Globals::Discount(count + 1)) / (1 - Globals::Discount());
	}
};

class RockSampleParticleUpperBound2: public ParticleUpperBound {
protected:
	const BaseRockSample* rs_model_;
public:
	RockSampleParticleUpperBound2(const BaseRockSample* model) :
		rs_model_(model) {
	}

	double Value(const State& state) const {
		const RockSampleState& rockstate =
			static_cast<const RockSampleState&>(state);
		double value = 0;
		for (int rock = 0; rock < rs_model_->num_rocks_; rock++)
			value += 10.0 * rs_model_->GetRock(&rockstate, rock)
				* Globals::Discount(
					Coord::ManhattanDistance(rs_model_->GetRobPos(&rockstate),
						rs_model_->rock_pos_[rock]));
		value += 10.0
			* Globals::Discount(rs_model_->size_ - rs_model_->GetX(&rockstate));
		return value;
	}
};

class RockSampleMDPParticleUpperBound: public ParticleUpperBound {
protected:
	const BaseRockSample* rs_model_;
	vector<ValuedAction> policy_;
public:
	RockSampleMDPParticleUpperBound(const BaseRockSample* model) :
		rs_model_(model) {
		policy_ = rs_model_->ComputeOptimalSamplingPolicy();
	}

	double Value(const State& state) const {
		return policy_[state.state_id].value;
	}
};

class RockSampleApproxParticleUpperBound: public ParticleUpperBound {
protected:
	const BaseRockSample* rs_model_;
public:
	RockSampleApproxParticleUpperBound(const BaseRockSample* model) :
		rs_model_(model) {
	}

	double Value(const State& state) const {
		double value = 0;
		double discount = 1.0;
		Coord rob_pos = rs_model_->GetRobPos(&state);
		vector<bool> visited(rs_model_->num_rocks_);
		while (true) {
			// Move to the nearest valuable rock and sample
			int shortest = 2 * rs_model_->size_;
			int id = -1;
			Coord rock_pos(-1, -1);
			for (int rock = 0; rock < rs_model_->num_rocks_; rock++) {
				int dist = Coord::ManhattanDistance(rob_pos,
					rs_model_->rock_pos_[rock]);
				if (CheckFlag(state.state_id, rock) && dist < shortest
					&& !visited[rock]) {
					shortest = dist;
					id = rock;
					rock_pos = rs_model_->rock_pos_[rock];
				}
			}

			if (id == -1)
				break;

			discount *= Globals::Discount(Coord::ManhattanDistance(rock_pos, rob_pos));
			value += discount * 10.0;
			visited[id] = true;
			rob_pos = rock_pos;
		}

		value += 10.0 * discount
			* Globals::Discount(rs_model_->size_ - rs_model_->GetX(&state));
		return value;
	}
};

ScenarioUpperBound* BaseRockSample::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	ScenarioUpperBound* bound = NULL;
	if (name == "TRIVIAL") {
		bound = new TrivialParticleUpperBound(this);
	} else if (name == "UB1") {
		bound = new RockSampleParticleUpperBound1(this);
	} else if (name == "UB2") {
		bound = new RockSampleParticleUpperBound2(this);
	} else if (name == "DEFAULT" || name == "MDP") {
		bound = new RockSampleMDPParticleUpperBound(this);
	} else if (name == "APPROX") {
		bound = new RockSampleApproxParticleUpperBound(this);
	} else {
		cerr << "Unsupported scenario upper bound: " << name << endl;
		exit(0);
	}
	return bound;
}

class RockSampleEastScenarioLowerBound : public ScenarioLowerBound {
private:
	const BaseRockSample* rs_model_;
	const Grid<int>& grid_;

public:
	RockSampleEastScenarioLowerBound(const DSPOMDP* model) :
		ScenarioLowerBound(model),
		rs_model_(static_cast<const BaseRockSample*>(model)),
		grid_(rs_model_->grid_) {
	}

	ValuedAction Value(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		return ValuedAction(Compass::EAST,
			10 * State::Weight(particles)
				* Globals::Discount(grid_.xsize() - rs_model_->GetX(particles[0]) - 1));
	}
};

class RockSampleMMAPStateScenarioLowerBound : public ScenarioLowerBound {
private:
	const BaseRockSample* rs_model_;
	const Grid<int>& grid_;
	vector<vector<int> > rock_order_;

public:
	RockSampleMMAPStateScenarioLowerBound(const DSPOMDP* model) :
		ScenarioLowerBound(model),
		rs_model_(static_cast<const BaseRockSample*>(model)),
		grid_(rs_model_->grid_) {
		const vector<ValuedAction> policy =
			rs_model_->ComputeOptimalSamplingPolicy();
		rock_order_ = vector<vector<int> >(policy.size());
		for (int s = 0; s < policy.size(); s++) {
			int cur = s;
			while (cur != policy.size() - 1) {
				int action = policy[cur].action;
				if (action == rs_model_->E_SAMPLE) {
					int rock = grid_(
						rs_model_->IndexToCoord(cur >> rs_model_->num_rocks_));
					if (rock < 0)
						exit(0);
					rock_order_[s].push_back(rock);
				}
				cur = rs_model_->NextState(cur, action);
			}
		}
	}

	ValuedAction Value(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		vector<double> expected_sampling_value = vector<double>(
			rs_model_->num_rocks_);
		int state = 0;
		Coord rob_pos(-1, -1);
		double total_weight = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			state = (1 << rs_model_->num_rocks_)
				* rs_model_->GetRobPosIndex(particle);
			rob_pos = rs_model_->GetRobPos(particle);
			for (int r = 0; r < rs_model_->num_rocks_; r++) {
				expected_sampling_value[r] += particle->weight
					* (CheckFlag(particle->state_id, r) ? 10 : -10);
			}
			total_weight += particle->weight;
		}

		for (int rock = 0; rock < rs_model_->num_rocks_; rock++)
			if (expected_sampling_value[rock] / total_weight > 0.0)
				SetFlag(state, rock);

		int action = -1;
		double value = 0;
		double discount = 1.0;
		for (int r = 0; r < rock_order_[state].size(); r++) {
			int rock = rock_order_[state][r];
			Coord rock_pos = rs_model_->rock_pos_[rock];

			if (action == -1) {
				if (rock_pos.x < rob_pos.x)
					action = Compass::WEST;
				else if (rock_pos.x > rob_pos.x)
					action = Compass::EAST;
				else if (rock_pos.y < rob_pos.y)
					action = Compass::SOUTH;
				else if (rock_pos.y > rob_pos.y)
					action = Compass::NORTH;
				else
					action = rs_model_->E_SAMPLE;
			}

			discount *= Globals::Discount(Coord::ManhattanDistance(rock_pos, rob_pos));
			value += discount * expected_sampling_value[rock];
			rob_pos = rock_pos;
		}

		if (action == -1)
			action = Compass::EAST;

		value += 10 * total_weight * discount
			* Globals::Discount(grid_.xsize() - rob_pos.x);

		return ValuedAction(action, value);
	}
};

class RockSampleENTScenarioLowerBound: public ScenarioLowerBound {
private:
	const BaseRockSample* rs_model_;
	const Grid<int>& grid_;

public:
	RockSampleENTScenarioLowerBound(const DSPOMDP* model) : // ENT: Explore Nearest in Thresholded State
		ScenarioLowerBound(model),
		rs_model_(static_cast<const BaseRockSample*>(model)),
		grid_(rs_model_->grid_) {
	}

	ValuedAction Value(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		vector<double> expected_sampling_value = vector<double>(
			rs_model_->num_rocks_);
		Coord rob_pos;
		double weight = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			rob_pos = rs_model_->GetRobPos(particle);
			weight += particle->weight;
			for (int i = 0; i < rs_model_->num_rocks_; i++) {
				expected_sampling_value[i] += particle->weight
					* (rs_model_->GetRock(particle, i) ? 10 : -10);
			}
		}

		int action = -1;
		double value = 0;
		double discount = 1.0;
		while (true) {
			// Move to the nearest valuable rock and sample
			int shortest = grid_.xsize() + grid_.ysize();
			int id = -1;
			Coord rock_pos(-1, -1);
			for (int rock = 0; rock < rs_model_->num_rocks_; rock++) {
				int dist = Coord::ManhattanDistance(rob_pos,
					rs_model_->rock_pos_[rock]);
				if (expected_sampling_value[rock] > 0.1 && dist < shortest) {
					shortest = dist;
					id = rock;
					rock_pos = rs_model_->rock_pos_[rock];
				}
			}
			/*
			 // Move to the most valuable rock and sample
			 int best = 0;
			 int id = -1;
			 Coord rock_pos(-1, -1);
			 for (int rock=0; rock<rs_model_->num_rocks_; rock++) {
			 double v = expected_sampling_value[rock] * Globals::Discount(Coord::ManhattanDistance(rob_pos, rs_model_->rock_pos_[rock]));
			 if (v > best) {
			 best = v;
			 id = rock;
			 rock_pos = rs_model_->rock_pos_[rock];
			 }
			 }
			 */

			if (id == -1)
				break;

			if (action == -1) {
				if (rock_pos.x < rob_pos.x)
					action = Compass::WEST;
				else if (rock_pos.x > rob_pos.x)
					action = Compass::EAST;
				else if (rock_pos.y < rob_pos.y)
					action = Compass::SOUTH;
				else if (rock_pos.y > rob_pos.y)
					action = Compass::NORTH;
				else
					action = rs_model_->E_SAMPLE;
			}

			discount *= Globals::Discount(Coord::ManhattanDistance(rock_pos, rob_pos));
			value += discount * expected_sampling_value[id];
			expected_sampling_value[id] = -1; // set to bad rock
			rob_pos = rock_pos;
		}

		if (action == -1)
			action = Compass::EAST;

		value += 10 * weight * discount * Globals::Discount(grid_.xsize() - rob_pos.x);

		return ValuedAction(action, value);
	}
};

ScenarioLowerBound* BaseRockSample::CreateScenarioLowerBound(string name, string
	particle_bound_name) const {
	if (name == "TRIVIAL") {
		return new TrivialParticleLowerBound(this);
	} else if (name == "DEFAULT" || name == "EAST") {
		// scenario_lower_bound_ = new BlindPolicy(this, Compass::EAST);
		return new RockSampleEastScenarioLowerBound(this);
	} else if (name == "RANDOM") {
		return new RandomPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "ENT") {
		return new RockSampleENTScenarioLowerBound(this);
	} else if (name == "MMAP") {
		return new RockSampleMMAPStateScenarioLowerBound(this);
	} else if (name == "MODE") {
		// scenario_lower_bound_ = new ModeStatePolicy(this);
		return NULL; // TODO
	} else {
		cerr << "Unsupported lower bound algorithm: " << name << endl;
		exit(0);
		return NULL;
	}
}

class RockSampleEastBeliefPolicy: public BeliefLowerBound {
private:
	const BaseRockSample* rs_model_;

public:
	RockSampleEastBeliefPolicy(const DSPOMDP* model, Belief* belief = NULL) :
		BeliefLowerBound(model, belief),
		rs_model_(static_cast<const BaseRockSample*>(model)) {
	}

	ValuedAction Value(const Belief* belief) const {
		const vector<State*>& particles =
			static_cast<const ParticleBelief*>(belief)->particles();
		return ValuedAction(Compass::EAST,
			Globals::Discount(rs_model_->size_ - rs_model_->GetX(particles[0]) - 1) * 10);
	}
};

BeliefLowerBound* BaseRockSample::CreateBeliefLowerBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefLowerBound(this);
	} else if (name == "DEFAULT" || name == "EAST") {
		return new RockSampleEastBeliefPolicy(this);
	} else {
		cerr << "Unsupported belief lower bound: " << name << endl;
		exit(0);
		return NULL;
	}
}

class RockSampleMDPBeliefUpperBound: public BeliefUpperBound {
protected:
	const BaseRockSample* rs_model_;
	vector<ValuedAction> policy_;
public:
	RockSampleMDPBeliefUpperBound(const BaseRockSample* model) :
		rs_model_(model) {
		policy_ = rs_model_->ComputeOptimalSamplingPolicy();
	}

	double Value(const Belief* belief) const {
		const vector<State*>& particles =
			static_cast<const ParticleBelief*>(belief)->particles();

		double value = 0;
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			value += particle->weight * policy_[particle->state_id].value;
		}

		return value;
	}
};

BeliefUpperBound* BaseRockSample::CreateBeliefUpperBound(string name) const {
	if (name == "TRIVIAL") {
		return new TrivialBeliefUpperBound(this);
	} else if (name == "DEFAULT" || name == "MDP") {
		return new RockSampleMDPBeliefUpperBound(this);
	} else {
		cerr << "Unsupported belief lower bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

class RockSamplePOMCPPrior: public POMCPPrior {
private:
	const BaseRockSample* rs_model_;

public:
	RockSamplePOMCPPrior(const DSPOMDP* model) :
		POMCPPrior(model),
		rs_model_(static_cast<const BaseRockSample*>(model)) {
	}

	void ComputePreference(const State& state) {
		legal_actions_.clear();
		preferred_actions_.clear();

		const RockSampleState& rockstate =
			static_cast<const RockSampleState&>(state);
		Coord rob = rs_model_->GetRobPos(&rockstate);

		// Step through history to compute useful information
		vector<int> counts(rs_model_->num_rocks_, 0);
		vector<int> measured(rs_model_->num_rocks_, 0);
		vector<bool> certain(rs_model_->num_rocks_, false);
		vector<bool> sampled(rs_model_->num_rocks_, false);
		Coord pos = rs_model_->start_pos_;
		for (int t = 0; t < history_.Size(); t++) {
			int action = history_.Action(t);
			OBS_TYPE obs = history_.Observation(t);

			if (action > rs_model_->E_SAMPLE) {
				int rock = action - 1 - rs_model_->E_SAMPLE;
				measured[rock]++;
				counts[rock] += (obs == rs_model_->E_GOOD) ? 1 : -1;

				if (rs_model_->rock_pos_[rock] == pos)
					certain[rock] = true;
			} else if (action < rs_model_->E_SAMPLE) {
				Coord next = pos + Compass::DIRECTIONS[action];
				if (rs_model_->grid_.Inside(next))
					pos = next;
			} else {
				int rock = rs_model_->grid_(pos);
				if (rock >= 0)
					sampled[rock] = true;
			}
		}

		// Legal actions: don't move outside, sense an unsampled rock, sample if
		// there is an unsampled rock at current position
		if (rob.y + 1 < rs_model_->size_)
			legal_actions_.push_back(Compass::NORTH);

		legal_actions_.push_back(Compass::EAST);

		if (rob.y - 1 >= 0)
			legal_actions_.push_back(Compass::SOUTH);

		if (rob.x - 1 >= 0)
			legal_actions_.push_back(Compass::WEST);

		for (int rock = 0; rock < rs_model_->num_rocks_; rock++)
			if (!sampled[rock])
				legal_actions_.push_back(rock + 1 + rs_model_->E_SAMPLE);

		int rock = rs_model_->grid_(rob);
		if (rock >= 0 && !sampled[rock]) {
			legal_actions_.push_back(rs_model_->E_SAMPLE);

			if (counts[rock] > 0) {
				preferred_actions_.push_back(rs_model_->E_SAMPLE);
				return;
			}
		}

		bool all_bad = true;
		bool north_interesting = false;
		bool south_interesting = false;
		bool west_interesting = false;
		bool east_interesting = false;

		for (int rock = 0; rock < rs_model_->num_rocks_; rock++) {
			if (!sampled[rock] && counts[rock] >= 0) {
				all_bad = false;

				if (rs_model_->rock_pos_[rock].y > rob.y)
					north_interesting = true;
				if (rs_model_->rock_pos_[rock].y < rob.y)
					south_interesting = true;
				if (rs_model_->rock_pos_[rock].x < rob.x)
					west_interesting = true;
				if (rs_model_->rock_pos_[rock].x > rob.x)
					east_interesting = true;
			}
		}

		// if all remaining rocks seem bad, then head east
		if (all_bad) {
			preferred_actions_.push_back(Compass::EAST);
			return;
		}

		if (north_interesting)
			preferred_actions_.push_back(Compass::NORTH);

		if (east_interesting)
			preferred_actions_.push_back(Compass::EAST);

		if (south_interesting)
			preferred_actions_.push_back(Compass::SOUTH);

		if (west_interesting)
			preferred_actions_.push_back(Compass::WEST);

		for (rock = 0; rock < rs_model_->num_rocks_; rock++) {
			if (!sampled[rock] && !certain[rock] && abs(counts[rock]) < 2
				&& measured[rock] < 5) {
				preferred_actions_.push_back(rock + 1 + rs_model_->E_SAMPLE);
			}
		}
	}
};

POMCPPrior* BaseRockSample::CreatePOMCPPrior(string name) const {
	if (name == "UNIFORM") {
		return new UniformPOMCPPrior(this);
	} else if (name == "DEFAULT" || name == "SHR") {
		return new RockSamplePOMCPPrior(this);
	} else {
		cerr << "Unsupported POMCP prior: " << name << endl;
		exit(1);
		return NULL;
	}
}

void BaseRockSample::PrintState(const State& state, ostream& out) const {
	out << endl;
	for (int x = 0; x < size_ + 2; x++)
		out << "# ";
	out << endl;
	for (int y = size_ - 1; y >= 0; y--) {
		out << "# ";
		for (int x = 0; x < size_; x++) {
			Coord pos(x, y);
			int rock = grid_(pos);
			int status = GetRock(&state, rock);
			if (GetRobPos(&state) == Coord(x, y))
				out << "R ";
			else if (rock >= 0)
				out << rock << (status ? "$" : "X");
			else
				out << ". ";
		}
		out << "#" << endl;
	}
	for (int x = 0; x < size_ + 2; x++)
		out << "# ";
	out << endl;
}

void BaseRockSample::PrintBelief(const Belief& belief, ostream& out) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief&>(belief).particles();

	vector<double> rock_probs(num_rocks_);
	vector<double> pos_probs(size_ * size_);
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		for (int rock = 0; rock < num_rocks_; rock++)
			rock_probs[rock] += CheckFlag(particle->state_id, rock)
				* particle->weight;
		pos_probs[particle->state_id >> num_rocks_] += particle->weight;
	}

	out << "Rock belief:";
	for (int rock = 0; rock < num_rocks_; rock++)
		out << " " << rock_probs[rock];
	out << endl;

	out << "Position belief:";
	for (int i = 0; i < pos_probs.size(); i++) {
		if (pos_probs[i] > 0)
			out << " " << IndexToCoord(i) << ":" << pos_probs[i];
	}
	out << endl;
}

void BaseRockSample::PrintAction(int action, ostream& out) const {
	if (action < E_SAMPLE)
		out << Compass::CompassString[action] << endl;
	if (action == E_SAMPLE)
		out << "Sample" << endl;
	if (action > E_SAMPLE)
		out << "Check " << (action - E_SAMPLE - 1) << endl;
}

State* BaseRockSample::Allocate(int state_id, double weight) const {
	RockSampleState* state = memory_pool_.Allocate();
	state->state_id = state_id;
	state->weight = weight;
	return state;
}

State* BaseRockSample::Copy(const State* particle) const {
	RockSampleState* state = memory_pool_.Allocate();
	*state = *static_cast<const RockSampleState*>(particle);
	state->SetAllocated();
	return state;
}

void BaseRockSample::Free(State* particle) const {
	memory_pool_.Free(static_cast<RockSampleState*>(particle));
}

int BaseRockSample::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

Belief* BaseRockSample::Tau(const Belief* belief, int action,
	OBS_TYPE obs) const {
	static vector<double> probs = vector<double>(NumStates());

	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double sum = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		RockSampleState* state = static_cast<RockSampleState*>(particle);
		int id = NextState(state->state_id, action);

		if (id == NumStates() - 1)
			continue; // ignore terminal state

		const RockSampleState& next = *(states_[id]);
		double p = state->weight * ObsProb(obs, next, action);
		probs[id] += p;
		sum += p;
	}

	vector<State*> new_particles;
	for (int i = 0; i < NumStates(); i++) {
		if (probs[i] > 0) {
			State* new_particle = Copy(states_[i]);
			new_particle->weight = probs[i] / sum;
			new_particles.push_back(new_particle);
			probs[i] = 0;
		}
	}

	if (new_particles.size() == 0) {
		cout << *belief << endl;
		exit(0);
	}

	return new ParticleBelief(new_particles, this, NULL, false);
}

void BaseRockSample::Observe(const Belief* belief, int action,
	map<OBS_TYPE, double>& obss) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	if (action <= E_SAMPLE) {
		RockSampleState* state = static_cast<RockSampleState*>(particles[0]);
		int id = NextState(state->state_id, action);
		if (id != NumStates() - 1)
			obss[E_NONE] = 1.0;
	} else { // Sense a rock
		for (int i = 0; i < particles.size(); i++) {
			State* particle = particles[i];
			RockSampleState* state = static_cast<RockSampleState*>(particle);
			int id = NextState(state->state_id, action);
			if (id == NumStates() - 1)
				continue; // ignore terminal state
			const RockSampleState& next = *(states_[id]);

			int rock = action - E_SAMPLE - 1;
			double distance = Coord::EuclideanDistance(GetRobPos(&next),
				rock_pos_[rock]);
			double efficiency = (1
				+ pow(2, -distance / half_efficiency_distance_)) * 0.5;

			if (GetRock(&next, rock)) {
				obss[E_GOOD] += state->weight * efficiency;
				if (efficiency != 1)
					obss[E_BAD] += state->weight * (1 - efficiency);
			} else {
				obss[E_BAD] += state->weight * efficiency;
				if (efficiency != 1)
					obss[E_GOOD] += state->weight * (1 - efficiency);
			}
		}
	}
}

double BaseRockSample::StepReward(const Belief* belief, int action) const {
	const vector<State*>& particles =
		static_cast<const ParticleBelief*>(belief)->particles();

	double value = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		value += particle->weight * Reward(particle->state_id, action);
	}

	return value;
}

int BaseRockSample::NumStates() const { // one dummy terminal state
	return ((grid_.xsize() * grid_.ysize()) << num_rocks_) + 1;
}

const State* BaseRockSample::GetState(int index) const {
	return states_[index];
}

int BaseRockSample::GetIndex(const State* state) const {
	return state->state_id;
}

int BaseRockSample::GetRobPosIndex(const State* state) const {
	return state->state_id >> num_rocks_;
}

Coord BaseRockSample::GetRobPos(const State* state) const {
	return grid_.GetCoord(state->state_id >> num_rocks_);
}

bool BaseRockSample::GetRock(const State* state, int rock) const {
	return CheckFlag(state->state_id, rock);
}

bool BaseRockSample::GetObservation(double rand_num,
	const RockSampleState& rockstate, int rock) const {
	double distance = Coord::EuclideanDistance(GetRobPos(&rockstate),
		rock_pos_[rock]);
	double efficiency = (1 + pow(2, -distance / half_efficiency_distance_))
		* 0.5;

	if (rand_num < efficiency)
		return GetRock(&rockstate, rock) & E_GOOD;
	else
		return !(GetRock(&rockstate, rock) & E_GOOD);
}

void BaseRockSample::SampleRock(State* state, int rock) const {
	UnsetFlag(state->state_id, rock);
}

int BaseRockSample::GetX(const State* state) const {
	return (state->state_id >> num_rocks_) % grid_.xsize();
}

void BaseRockSample::IncX(State* state) const {
	state->state_id += (1 << num_rocks_);
}

void BaseRockSample::DecX(State* state) const {
	state->state_id -= (1 << num_rocks_);
}

int BaseRockSample::GetY(const State* state) const {
	return (state->state_id >> num_rocks_) / grid_.xsize();
}

void BaseRockSample::IncY(State* state) const {
	state->state_id += (1 << num_rocks_) * grid_.xsize();
}

void BaseRockSample::DecY(State* state) const {
	state->state_id -= (1 << num_rocks_) * grid_.xsize();
}

Coord BaseRockSample::IndexToCoord(int pos) const {
	return Coord(pos % grid_.xsize(), pos / grid_.xsize());
}

int BaseRockSample::CoordToIndex(Coord c) const {
	return c.y * grid_.xsize() + c.x;
}

int BaseRockSample::NextState(int s, int a) const {
	if (s == NumStates() - 1)
		return NumStates() - 1;

	Coord rob_pos = IndexToCoord(s >> num_rocks_);
	if (a < E_SAMPLE) {
		rob_pos += Compass::DIRECTIONS[a];
		if (grid_.Inside(rob_pos)) {
			int next_pos = CoordToIndex(rob_pos);
			return s + ((next_pos - (s >> num_rocks_)) << num_rocks_);
		} else {
			return rob_pos.x == grid_.xsize() ? (NumStates() - 1) : s;
		}
	} else if (a == E_SAMPLE) {
		int next = s;
		Coord rob_pos = IndexToCoord(s >> num_rocks_);
		int rock = grid_(rob_pos);
		if (rock >= 0)
			UnsetFlag(next, rock);
		return next;
	} else
		return s;
}

double BaseRockSample::Reward(int s, int a) const {
	if (s == NumStates() - 1)
		return 0;

	if (a < E_SAMPLE) {
		Coord rob_pos = IndexToCoord(s >> num_rocks_);
		rob_pos += Compass::DIRECTIONS[a];
		if (grid_.Inside(rob_pos)) {
			return 0;
		} else
			return rob_pos.x == grid_.xsize() ? 10 : -100;
	} else if (a == E_SAMPLE) {
		Coord rob_pos = IndexToCoord(s >> num_rocks_);
		int rock = grid_(rob_pos);
		if (rock >= 0) {
			bool valuable = CheckFlag(s, rock);
			if (valuable)
				return 10;
			else
				return -10;
		}
		return -100;
	} else
		return 0;
}

void BaseRockSample::InitializeTransitions() {
	int num_states = NumStates(), num_actions = NumActions();
	transition_probabilities_.resize(num_states);
	for (int s = 0; s < num_states; s++) {
		transition_probabilities_[s].resize(num_actions);
		for (int a = 0; a < num_actions; a++) {
			State state;
			state.state_id = NextState(s, a);
			state.weight = 1.0;
			transition_probabilities_[s][a].push_back(state);
		}
	}
}

const vector<State>& BaseRockSample::TransitionProbability(int s, int a) const {
	return transition_probabilities_[s][a];
}

vector<ValuedAction>& BaseRockSample::ComputeOptimalSamplingPolicy() const {
	int num_states = NumStates();
	if (mdp_policy_.size() == num_states)
		return mdp_policy_;

	mdp_policy_ = vector<ValuedAction>(num_states);
	for (int s = 0; s < num_states; s++) {
		mdp_policy_[s].value = 0;
	}

	clock_t start = clock();
	cerr << "Computing optimal MDP policy...";
	vector<ValuedAction> next_policy = vector<ValuedAction>(num_states);
	// Using precomputed transition saves around 60% of the time
	vector<vector<int> > transition = vector<vector<int> >(num_states);
	for (int s = 0; s < num_states; s++) {
		transition[s].resize(E_SAMPLE + 1);
		for (int a = 0; a <= E_SAMPLE; a++) // Sensing actions not needed
			transition[s][a] = NextState(s, a);
	}

	int iter = 0;
	double diff;
	while (true) {
		for (int s = 0; s < num_states; s++) {
			next_policy[s].action = -1;
			next_policy[s].value = Globals::NEG_INFTY;

			for (int a = 0; a <= E_SAMPLE; a++) {
				double v = Reward(s, a)
					+ Globals::Discount() * mdp_policy_[transition[s][a]].value;

				if (v > next_policy[s].value) {
					next_policy[s].value = v;
					next_policy[s].action = a;
				}
			}
		}

		diff = 0;
		for (int s = 0; s < num_states; s++) {
			diff += fabs(next_policy[s].value - mdp_policy_[s].value);
			mdp_policy_[s] = next_policy[s];
		}

		iter++;
		if (diff < 0.001)
			break;
	}
	cerr << "Done [" << iter << " iters, tol = " << diff << ", "
		<< (double) (clock() - start) / CLOCKS_PER_SEC << "s]!" << endl;

	return mdp_policy_;
}

RockSampleState* BaseRockSample::MajorityRockSampleState(
	const vector<State*>& particles) const {
	static vector<double> probs = vector<double>(num_rocks_);

	int state = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		state = (1 << num_rocks_) * GetRobPosIndex(particle);
		for (int i = 0; i < num_rocks_; i++) {
			probs[i] += particle->weight * (2 * GetRock(particle, i) - 1);
		}
	}

	for (int i = 0; i < probs.size(); i++) {
		if (probs[i] > 0)
			SetFlag(state, i);
		probs[i] = 0.0;
	}

	return new RockSampleState(state);
}

} // namespace despot
