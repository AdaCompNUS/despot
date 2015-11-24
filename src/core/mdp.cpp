#include <despot/core/mdp.h>

using namespace std;

namespace despot {

MDP::~MDP() {
}

void MDP::ComputeOptimalPolicyUsingVI() {
	if (policy_.size() != 0)
		return;

	int num_states = NumStates(), num_actions = NumActions();
	policy_.resize(num_states);

	policy_ = vector<ValuedAction>(num_states);
	for (int s = 0; s < num_states; s++) {
		policy_[s].value = 0;
	}

	clock_t start = clock();
	logi << "[MDP::ComputeOptimalPolicyUsingVI] Computing optimal MDP policy...";
	vector<ValuedAction> next_policy = vector<ValuedAction>(num_states);

	int iter = 0;
	double diff;
	while (true) {
		for (int s = 0; s < num_states; s++) {
			next_policy[s].action = -1;
			next_policy[s].value = Globals::NEG_INFTY;

			for (int a = 0; a < num_actions; a++) {
				double v = Reward(s, a);
				const vector<State>& transition = TransitionProbability(s, a);
				for (int i = 0; i < transition.size(); i++) {
					const State& next = transition[i];
					assert(next.state_id >= 0);
					v += next.weight * Globals::Discount()
						* policy_[next.state_id].value;
				}

				if (v > next_policy[s].value) {
					next_policy[s].value = v;
					next_policy[s].action = a;
				}
			}
		}

		diff = 0;
		for (int s = 0; s < num_states; s++) {
			diff += fabs(next_policy[s].value - policy_[s].value);
			policy_[s] = next_policy[s];
		}

		iter++;
		if (diff < 1E-6)
			break;
	}
	logi << "Done [" << iter << " iters, tol = " << diff << ", "
		<< (double) (clock() - start) / CLOCKS_PER_SEC << "s]!" << endl;
}

void MDP::ComputeBlindAlpha() {
	int num_states = NumStates(), num_actions = NumActions();

	blind_alpha_.resize(num_actions);

	for (int action = 0; action < num_actions; action++) {
		vector<double> cur(num_states), prev(num_states);

		double min = Globals::POS_INFTY;
		for (int s = 0; s < num_states; s++) {
			double reward = Reward(s, action);
			if (reward < min)
				min = reward;
		}

		for (int s = 0; s < num_states; s++)
			cur[s] = min / (1 - Globals::Discount());

		double tol = 0;
		int iter = 0;
		for (iter = 0; iter < 1000; iter++) {
			tol = 0;

			for (int s = 0; s < num_states; s++)
				prev[s] = cur[s];

			for (int s = 0; s < num_states; s++) {
				cur[s] = 0;
				const vector<State>& transition = TransitionProbability(s,
					action);
				for (int i = 0; i < transition.size(); i++) {
					const State& next = transition[i];
					assert(next.state_id >= 0);
					cur[s] += next.weight * prev[next.state_id];
				}
				cur[s] = Reward(s, action) + Globals::Discount() * cur[s];

				tol += abs(cur[s] - prev[s]);
			}

			if (tol < 0.0001) {
				iter++;
				break;
			}
		}
		logi << "[MDP::ComputeBlindAlpha] Tol(alpha_" << action << ") after " << iter << " iters = "
			<< tol << endl;

		blind_alpha_[action] = cur;
	}
}

double MDP::ComputeActionValue(const ParticleBelief* belief,
	const StateIndexer& indexer, int action) const {
	assert(blind_alpha_.size() != 0);

	const vector<State*> particles = belief->particles();
	double value = 0;
	for (int i = 0; i < particles.size(); i++) {
		State* particle = particles[i];
		value += particle->weight
			* blind_alpha_[action][indexer.GetIndex(particle)];
	}

	return value;
}

const vector<ValuedAction>& MDP::policy() const {
	return policy_;
}

} // namespace despot
