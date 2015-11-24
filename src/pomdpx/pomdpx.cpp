#include <despot/pomdpx/pomdpx.h>
#include <despot/solver/pomcp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * POMDPXState class
 * =============================================================================*/

POMDPXState::POMDPXState() {
}

POMDPXState::POMDPXState(vector<int> aIndex) {
	vec_id = aIndex;
}

POMDPXState::~POMDPXState() {
}

string POMDPXState::text() const {
	ostringstream oss;
	POMDPX::current_->PrintState(*this, oss);
	string str = oss.str();
	return str.substr(0, str.length() - 1);
}

/* =============================================================================
 * POMDPX class
 * =============================================================================*/

POMDPX* POMDPX::current_ = NULL;
int POMDPX::STATE_NUM_THRESHOLD = 1000000;

POMDPX::POMDPX() {
	current_ = this;
}

POMDPX::POMDPX(string params_file) {
	current_ = this;
	parser_ = new Parser(params_file);

	max_reward_action_ = parser_->ComputeMaxRewardAction();
	logi << "Max reward action = " << max_reward_action_ << " ";
	min_reward_action_ = parser_->ComputeMinRewardAction();
	logi << "Min reward action = " << min_reward_action_ << " ";
	logi << "#State / log2(#State) = " << parser_->NumStates() << " / "
		<< parser_->LogNumStates() << endl;
	logi << "#InitState / log2(#InitState) = " << parser_->NumInitialStates()
		<< " / " << parser_->LogNumInitialStates() << endl;
	logi << "#Action = " << parser_->NumActions() << endl;
	logi << "#Obs / log2(#Obs) = " << parser_->NumObservations() << " / "
		<< parser_->LogNumObservations() << endl;

	is_small_ = (parser_->LogNumStates() <= 15);

	if (is_small_) {
		InitStates();
		InitTransitions();
		InitRewards(); // Must be called after initializing transitions

		for (int a = 0; a < NumActions(); a++) {
			double min_reward = Globals::POS_INFTY;

			for (int s = 0; s < NumStates(); s++) {
				if (rewards_[s][a] < min_reward) {
					min_reward = rewards_[s][a];
				}
			}

			if (min_reward > min_reward_action_.value) {
				min_reward_action_ = ValuedAction(a, min_reward);
			}
		}
	}
}

void POMDPX::PrintModel(ostream& out) const {
	out << "Parser pointer = " << parser_ << endl;
	out << "is_small = " << is_small_ << endl;

	out << "Min reward action = " << min_reward_action_ << endl;
	out << "Max reward action = " << max_reward_action_ << endl;

	out << "States = " << states_ << endl;
	out << "Transitions = " << transition_probabilities_ << endl;
	out << "Rewards = " << rewards_ << endl;
	out << "MemoryPool = " << &memory_pool_ << endl;
}

bool POMDPX::NoisyStep(State& s, double random_num, int action) const {
	POMDPXState& state = static_cast<POMDPXState&>(s);

	parser_->GetNoisyNextState(state.vec_id, action, random_num);

	return parser_->IsTerminalState(state.vec_id);
}

bool POMDPX::Step(State& s, double random_num, int action, double& reward,
	OBS_TYPE& obs) const {
	POMDPXState& state = static_cast<POMDPXState&>(s);

	parser_->GetNextState(state.vec_id, action, random_num);
	reward = parser_->GetReward(action); // Prev state and curr state set in GetNextState
	obs = parser_->GetObservation(state.vec_id, action, random_num);

	return parser_->IsTerminalState(state.vec_id);
}

int POMDPX::NumActions() const {
	return parser_->NumActions();
}

int POMDPX::NumStates() const {
	return parser_->NumStates();
}

double POMDPX::ObsProb(OBS_TYPE obs, const State& s, int a) const {
	const POMDPXState& state = static_cast<const POMDPXState&>(s);

	return parser_->ObsProb(obs, state.vec_id, a);
}

State* POMDPX::CreateStartState(string type) const {
	double random_value = Random::RANDOM.NextDouble();
	return new POMDPXState(parser_->ComputeState(random_value));
}

// The implementation mostly follows that of ParticleBelief, but handles
// the problem of particle depletion different
class POMDPXBelief: public ParticleBelief {
protected:
	int max_iter_;
	const POMDPX* model_;
public:
	POMDPXBelief(vector<State*> particles, int max_iter,
		const DSPOMDP* model, Belief* prior = NULL) :
		ParticleBelief(particles, model, prior),
		max_iter_(max_iter),
		model_(static_cast<const POMDPX*>(model)) {
	}

	void Update(int action, OBS_TYPE obs) {
		history_.Add(action, obs);

		vector<State*> updated;
		double reward;
		OBS_TYPE o;

		// Update particles by looping through the particles for multiple iterations
		for (int i = 0; i < max_iter_; i++) {
			for (int j = 0; j < particles_.size(); j++) {
				State* particle = particles_[j];
				State* copy = model_->Copy(particle);

				bool terminal = model_->Step(*copy, Random::RANDOM.NextDouble(),
					action, reward, o);
				double prob = model_->ObsProb(obs, *copy, action);

				if (!terminal && prob) { // Terminal state is not required to be explicitly represented and may not have any observation
					copy->weight *= prob;
					updated.push_back(copy);
				} else {
					model_->Free(copy);
				}

				if (updated.size() == particles_.size())
					break;
			}
			if (updated.size() == particles_.size()) {
				logi << "[POMDPXBelief::Update] " << i
					<< " iterations for normal simulations" << endl;
				break;
			}
		}

		// Perform at most max_iter_ noisy iterations to generate particles, or keep
		// inconsistent particles after that
		if (updated.size() != particles_.size()) {
			for (int i = 0; i < max_iter_ + 1; i++) {
				for (int j = 0; j < particles_.size(); j++) {
					State* particle = particles_[j];
					State* copy = model_->Copy(particle);

					bool terminal = model_->NoisyStep(*copy,
						Random::RANDOM.NextDouble(), action);
					double prob = model_->ObsProb(obs, *copy, action);

					if (i == max_iter_ && prob < 1E-6) // NOTE: never kill a particle at the last iteration
						prob = 1E-6;

					if (!terminal && prob) {
						copy->weight *= prob;
						updated.push_back(copy);
					} else {
						model_->Free(copy);
					}

					if (updated.size() == particles_.size())
						break;
				}
				if (updated.size() == particles_.size()) {
					logi << "[POMDPXBelief::Update] " << i
						<< " iterations for noisy simulations" << endl;
					break;
				}
			}
		}

		for (int i = 0; i < particles_.size(); i++)
			model_->Free(particles_[i]);

		particles_ = updated;

		double total_weight = State::Weight(particles_);
		double weight_square_sum = 0;
		for (int i = 0; i < particles_.size(); i++) {
			State* particle = particles_[i];
			particle->weight /= total_weight;
			weight_square_sum += particle->weight * particle->weight;
		}

		// Resample if the effective number of particles is "small"
		double num_effective_particles = 1.0 / weight_square_sum;
		if (num_effective_particles < num_particles_ / 20.0) { // NOTE: small particles may be removed...
			logi << "[POMDPXBelief::Update] Resampling " << num_particles_
				<< " as effective number of particles = "
				<< num_effective_particles << endl;
			vector<State*> new_belief = Belief::Sample(num_particles_,
				particles_, model_);
			for (int i = 0; i < particles_.size(); i++)
				model_->Free(particles_[i]);

			particles_ = new_belief;
		}
	}
};

vector<State*> POMDPX::ExactInitialParticleSet() const {
	vector<State*> particles;

	for (int s = 0; s < NumStates(); s++) {
		POMDPXState* state = static_cast<POMDPXState*>(Allocate(s,
			parser_->InitialWeight(states_[s]->vec_id)));
		if (state->weight == 0) {
			Free(state);
		} else {
			state->vec_id = states_[s]->vec_id;
			particles.push_back(state);
		}
	}

	return particles;
}

vector<State*> POMDPX::ApproxInitialParticleSet() const {
	vector<State*> particles;

	logi
		<< "[POMDPX::ApproximatePrior] Drawing 1000 random particles from initial belief."
		<< endl;
	for (int i = 0; i < 1000; i++) {
		double random_value = Random::RANDOM.NextDouble();
		POMDPXState* state = static_cast<POMDPXState*>(Allocate(-1, 0.001));
		state->vec_id = parser_->ComputeState(random_value);
		particles.push_back(state);
	}

	return particles;
}

Belief* POMDPX::InitialBelief(const State* start, string type) const {
	vector<State*> particles;
	if (is_small_ && parser_->LogNumInitialStates() < 13) { // Small problem
		particles = ExactInitialParticleSet();
	} else {
		particles = ApproxInitialParticleSet();
	}

	Belief* belief = NULL;
	if (type == "noisy")
		belief = new POMDPXBelief(particles, 10, this);
	else // if (type == "particle")
		belief = new ParticleBelief(particles, this);
	return belief;
}

void POMDPX::InitStates() {
	assert(is_small_);

	double start = get_time_second();
	states_.resize(NumStates());

	for (int s = 0; s < states_.size(); s++) {
		POMDPXState* state = static_cast<POMDPXState*>(Allocate(s, 0));
		state->vec_id = parser_->ComputeState(s);
		states_[s] = state;
	}

	for (int s = 0; s < states_.size(); s++)
		assert(GetIndex(states_[s]) == s);

	logi << "[POMDPX::InitState] Initialized state table in "
		<< (get_time_second() - start) << "s" << endl;
}

void POMDPX::InitTransitions() {
	assert(is_small_);

	double start = get_time_second();
	int num_states = NumStates(), num_actions = NumActions();
	transition_probabilities_.resize(num_states);
	for (int s = 0; s < num_states; s++) {
		vector<int> state = parser_->ComputeState(s);
		transition_probabilities_[s].resize(num_actions);
		for (int a = 0; a < num_actions; a++) {
			vector<pair<vector<int>, double> > best =
				parser_->ComputeTopTransitions(state, a, 500000 / num_states);
			for (int i = 0; i < best.size(); i++) {
				const pair<vector<int>, double>& pair = best[i];
				transition_probabilities_[s][a].push_back(
					State(parser_->ComputeIndex(pair.first), pair.second));
			}
		}
	}
	logi << "[POMDPX::InitTransitions] Initialized transition table in "
		<< (get_time_second() - start) << "s." << endl;
}

void POMDPX::PrintTransitions() {
	int num_states = NumStates(), num_actions = NumActions();
	for (int s = 0; s < num_states; s++) {
		cout << "State " << s << endl;
		vector<int> state = parser_->ComputeState(s);
		parser_->PrintState(state);
		transition_probabilities_[s].resize(num_actions);
		for (int a = 0; a < num_actions; a++) {
			cout << "Action " << a << " -> " << endl;
			for (int i = 0; i < transition_probabilities_[s][a].size(); i++) {
				const State& state = transition_probabilities_[s][a][i];
				cout << "w = " << state.weight << endl;
				PrintState(*states_[state.state_id]);
			}
			cout << endl;
		}
	}
}

void POMDPX::InitRewards() {
	assert(is_small_);

	rewards_.resize(NumStates());

	for (int s = 0; s < NumStates(); s++) {
		rewards_[s].resize(NumActions());
		for (int a = 0; a < NumActions(); a++) {
			rewards_[s][a] = 0.0;
			const vector<State>& transition = TransitionProbability(s, a);
			for (int i = 0; i< transition.size(); i++) {
				const State& next = transition[i];
				rewards_[s][a] += next.weight
					* parser_->GetReward(states_[s]->vec_id,
						states_[next.state_id]->vec_id, a);
			}
		}
	}
}

int POMDPX::GetIndex(const State* state) const {
	const vector<int>& vec = static_cast<const POMDPXState*>(state)->vec_id;
	return parser_->ComputeIndex(vec);
}

const State* POMDPX::GetState(const int index) const {
	return states_[index];
}

const vector<State>& POMDPX::TransitionProbability(int s, int a) const {
	assert(is_small_);

	return transition_probabilities_[s][a];
}

double POMDPX::Reward(int s, int a) const {
	assert(is_small_);

	return rewards_[s][a];
}

void POMDPX::ComputeDefaultActions(string type) const {
	if (type == "MDP") {
		const_cast<POMDPX*>(this)->ComputeOptimalPolicyUsingVI();
		int num_states = NumStates();
		default_action_.resize(num_states);
		for (int s = 0; s < num_states; s++) {
			default_action_[s] = policy_[s].action;
		}
	} else {
		cerr << "Unsupported default action type " << type << endl;
		exit(0);
	}
}

void POMDPX::PrintDefaultActions() {
	int num_states = NumStates();
	for (int s = 0; s < num_states; s++) {
		cout << "s = " << s << " " << states_[s]->vec_id << " " << policy_[s]
			<< endl;
		PrintState(*states_[s]);
	}
}

class POMDPXGreedyActionPolicy: public Policy {
private:
	const POMDPX* pomdpx_model_;
public:
	POMDPXGreedyActionPolicy(const DSPOMDP* model, ParticleLowerBound* bound) :
		Policy(model, bound),
		pomdpx_model_(static_cast<const POMDPX*>(model)) {
	}

	int Action(const vector<State*>& particles, RandomStreams& streams,
		History& history) const {
		int bestAction = 0;
		double maxReward = Globals::NEG_INFTY;
		for (int action = 0; action < pomdpx_model_->NumActions(); action++) {
			double reward = 0;
			for (int i = 0; i < particles.size(); i++) {
				State* particle = particles[i];
				POMDPXState* state = static_cast<POMDPXState*>(particle);
				reward += state->weight
					* pomdpx_model_->parser_->GetReward(state->vec_id,
						state->vec_id, action);
			}
			if (reward > maxReward) {
				maxReward = reward;
				bestAction = action;
			}
		}
		return bestAction;
	}
};

ScenarioLowerBound* POMDPX::CreateScenarioLowerBound(string name,
	string particle_bound_name) const {
	if (!is_small_ && (name == "MAJORITY" || name == "MODE")) {
		return new TrivialParticleLowerBound(this);
	}

	const StatePolicy* policy = this;
	const StateIndexer* indexer = this;

	if (name == "TRIVIAL" || name == "DEFAULT") {
		return new TrivialParticleLowerBound(this);
	} else if (name == "RANDOM") {
		return new RandomPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "POMCP") {
		return new POMCPScenarioLowerBound(this, new UniformPOMCPPrior(this));
	} else if (name == "MAJORITY") {
		ComputeDefaultActions("MDP");
		return new MajorityActionPolicy(this, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "GREEDY") {
		return new POMDPXGreedyActionPolicy(this,
			CreateParticleLowerBound(particle_bound_name));
	} else if (name == "MODE") {
		ComputeDefaultActions("MDP");
		return new ModeStatePolicy(this, *indexer, *policy,
			CreateParticleLowerBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario lower bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

ParticleUpperBound* POMDPX::CreateParticleUpperBound(string name) const {
	if (!is_small_) {
		return new TrivialParticleUpperBound(this);
	}

	const StateIndexer* indexer = this;
	const MDP* mdp = this;

	if (name == "TRIVIAL") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP" || name == "DEFAULT") {
		return new MDPUpperBound(mdp, *indexer);
	} else {
		cerr << "Unsupported particle upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

ScenarioUpperBound* POMDPX::CreateScenarioUpperBound(string name,
	string particle_bound_name) const {
	if (!is_small_) {
		return new TrivialParticleUpperBound(this);
	}

	const StateIndexer* indexer = this;

	if (name == "TRIVIAL") {
		return new TrivialParticleUpperBound(this);
	} else if (name == "MDP" || name == "DEFAULT") {
		return new MDPUpperBound(this, *this);
	} else if (name == "LOOKAHEAD") {
		return new LookaheadUpperBound(this, *indexer,
			CreateParticleUpperBound(particle_bound_name));
	} else {
		cerr << "Unsupported scenario upper bound: " << name << endl;
		exit(1);
		return NULL;
	}
}

void POMDPX::PrintState(const State& s, ostream& out) const {
	const POMDPXState& state = static_cast<const POMDPXState&>(s);

	parser_->PrintState(state.vec_id, out);
}

void POMDPX::PrintBelief(const Belief& belief, ostream& out) const {
}

void POMDPX::PrintObs(const State& state, OBS_TYPE obs, ostream& out) const {
	parser_->PrintObs(obs, out);
}

void POMDPX::PrintAction(int action, ostream& out) const {
	parser_->PrintAction(action, out);
}

State* POMDPX::Allocate(int state_id, double weight) const {
	POMDPXState* particle = memory_pool_.Allocate();
	particle->state_id = state_id;
	particle->weight = weight;
	return particle;
}

State* POMDPX::Copy(const State* particle) const {
	POMDPXState* new_particle = memory_pool_.Allocate();
	*new_particle = *static_cast<const POMDPXState*>(particle);
	new_particle->SetAllocated();
	return new_particle;
}

void POMDPX::Free(State* particle) const {
	memory_pool_.Free(static_cast<POMDPXState*>(particle));
}

int POMDPX::NumActiveParticles() const {
	return memory_pool_.num_allocated();
}

OBS_TYPE POMDPX::GetPOMDPXObservation(map<string, string>& observe) {
	return parser_->GetPOMDPXObservation(observe);
}

const string& POMDPX::GetActionName() {
	return parser_->GetActionName();
}

const string& POMDPX::GetEnumedAction(int action) {
	return parser_->GetEnumedAction(action);
}

// Don't copy memory pool! Allocated particles will be in two pools.
DSPOMDP* POMDPX::MakeCopy() const {
	POMDPX* pomdpx = new POMDPX();

	pomdpx->parser_ = this->parser_;
	pomdpx->is_small_ = this->is_small_;
	pomdpx->min_reward_action_ = this->min_reward_action_;
	pomdpx->max_reward_action_ = this->max_reward_action_;

	pomdpx->states_ = this->states_;
	pomdpx->transition_probabilities_ = this->transition_probabilities_;
	pomdpx->rewards_ = this->rewards_;

	pomdpx->default_action_ = this->default_action_;

	// pomdpx->PrintModel();
	// this->PrintModel();

	return pomdpx;
}

} // namespace despot
