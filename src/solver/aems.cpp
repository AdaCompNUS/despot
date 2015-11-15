#include <despot/solver/aems.h>

using namespace std;

namespace despot {

AEMS::AEMS(const DSPOMDP* model, BeliefLowerBound* lower_bound,
	BeliefUpperBound* upper_bound, Belief* belief) :
	Solver(model, belief),
	root_(NULL),
	lower_bound_(lower_bound),
	upper_bound_(upper_bound) {
	model_ = static_cast<const BeliefMDP*>(model);
	reuse_ = false;
}

ValuedAction AEMS::Search() {
	if (root_ == NULL) {
		root_ = new VNode(belief_->MakeCopy());
		InitLowerBound(root_, lower_bound_, history_);
		InitUpperBound(root_, upper_bound_, history_);
	}

	statistics_ = SearchStatistics();
	statistics_.num_particles_before_search = model_->NumActiveParticles();

	model_->PrintBelief(*belief_); //TODO: check and remove
	// cout << *belief_ << endl;
	clock_t begin = clock();
	statistics_.initial_lb = root_->lower_bound();
	statistics_.initial_ub = root_->upper_bound();

	int num_active_particles = model_->NumActiveParticles();
	do {
		VNode* promising_node = FindMaxApproxErrorLeaf(root_);
		assert(promising_node->IsLeaf());

		int hist_size = history_.Size();
		clock_t start = clock();
		Expand(promising_node, lower_bound_, upper_bound_, model_, history_);
		statistics_.time_node_expansion += (clock() - start) / CLOCKS_PER_SEC;
		Backup(promising_node);
		history_.Truncate(hist_size);

		if (promising_node->depth() > statistics_.longest_trial_length) {
			statistics_.longest_trial_length = promising_node->depth();
		}

		statistics_.num_trials++;
		statistics_.num_expanded_nodes++;
	} while ((double) (clock() - begin) / CLOCKS_PER_SEC
		< Globals::config.time_per_move
		&& (root_->upper_bound() - root_->lower_bound()) > 1e-6);

	statistics_.num_tree_particles = model_->NumActiveParticles()
		- num_active_particles;
	statistics_.num_tree_nodes = root_->Size();
	statistics_.num_policy_nodes = root_->PolicyTreeSize();
	statistics_.num_particles_after_search = model_->NumActiveParticles();

	statistics_.final_lb = root_->lower_bound();
	statistics_.final_ub = root_->upper_bound();
	statistics_.time_search = (double) (clock() - begin) / CLOCKS_PER_SEC;

	logi << "[AEMS::Search]" << statistics_ << endl;

	ValuedAction astar = OptimalAction(root_);
	//delete root_;
	return astar;
}

ValuedAction AEMS::OptimalAction(const VNode* vnode) {
	ValuedAction astar(-1, Globals::NEG_INFTY);
	for (int action = 0; action < vnode->children().size(); action++) {
		const QNode* qnode = vnode->Child(action);
		if (qnode->lower_bound() > astar.value) {
			astar = ValuedAction(action, qnode->lower_bound());
		}
	}
	return astar;
}

VNode* AEMS::FindMaxApproxErrorLeaf(VNode* root) {
	double bestAE = Globals::NEG_INFTY;
	VNode* bestNode = NULL;
	FindMaxApproxErrorLeaf(root, 1.0, bestAE, bestNode);
	return bestNode;
}

void AEMS::FindMaxApproxErrorLeaf(VNode* vnode, double likelihood,
	double& bestAE, VNode*& bestNode) {
	if (vnode->IsLeaf()) {
		double curAE = likelihood * vnode->likelihood * Globals::Discount(vnode->depth())
			* (vnode->upper_bound() - vnode->lower_bound());
		if (curAE > bestAE) {
			bestAE = curAE;
			bestNode = vnode;
		}
	} else {
		for (int a = 0; a < vnode->children().size(); a++) {
			FindMaxApproxErrorLeaf(vnode->Child(a), likelihood, bestAE,
				bestNode);
		}
	}
}

void AEMS::FindMaxApproxErrorLeaf(QNode* qnode, double likelihood,
	double& bestAE, VNode*& bestNode) {
	likelihood *= Likelihood(qnode);

	map<OBS_TYPE, VNode*>& children = qnode->children();
	for (map<OBS_TYPE, VNode*>::iterator it = children.begin();
			it != children.end(); it++) {
		VNode* vnode = it->second;
		FindMaxApproxErrorLeaf(vnode, likelihood, bestAE, bestNode);
	}
}

double AEMS::Likelihood(QNode* qnode) {
	return AEMS2Likelihood(qnode);
}

double AEMS::AEMS2Likelihood(QNode* qnode) {
	VNode* vnode = qnode->parent();
	QNode* qstar = NULL;
	for (int action = 0; action < vnode->children().size(); action++) {
		QNode* child = vnode->Child(action);

		if (qstar == NULL || child->upper_bound() > qstar->upper_bound())
			qstar = child;
	}

	return qstar == qnode;
}

void AEMS::Update(VNode* vnode) {
	if (vnode->IsLeaf())
		return;

	double lower = Globals::NEG_INFTY;
	double upper = Globals::NEG_INFTY;

	for (int action = 0; action < vnode->children().size(); action++) {
		QNode* qnode = vnode->Child(action);

		lower = max(lower, qnode->lower_bound());
		upper = max(upper, qnode->upper_bound());
	}

	if (lower > vnode->lower_bound())
		vnode->lower_bound(lower);
	if (upper < vnode->upper_bound())
		vnode->upper_bound(upper);
}

void AEMS::Update(QNode* qnode) {
	double lower = qnode->step_reward;
	double upper = qnode->step_reward;

	map<OBS_TYPE, VNode*>& children = qnode->children();
	for (map<OBS_TYPE, VNode*>::iterator it = children.begin();
			it != children.end(); it++) {
		VNode* vnode = it->second;

		lower += Globals::Discount() * vnode->likelihood * vnode->lower_bound();
		upper += Globals::Discount() * vnode->likelihood * vnode->upper_bound();
	}

	if (lower > qnode->lower_bound())
		qnode->lower_bound(lower);
	if (upper < qnode->upper_bound())
		qnode->upper_bound(upper);
}

void AEMS::Backup(VNode* vnode) {
	int iter = 0;
	logd << "- Backup " << vnode << " at depth " << vnode->depth() << endl;
	while (true) {
		logd << " Iter " << (iter++) << " " << vnode << endl;

		Update(vnode);
		logd << " Updated vnode " << vnode << endl;

		QNode* parentq = vnode->parent();
		if (parentq == NULL)
			break;

		Update(parentq);
		logd << " Updated Q-node to (" << parentq->lower_bound() << ", "
			<< parentq->upper_bound() << ")" << endl;

		vnode = parentq->parent();
	}
	logd << "* Backup complete!" << endl;
}

void AEMS::InitLowerBound(VNode* vnode, BeliefLowerBound* lower_bound,
	History& history) {
	double value = lower_bound->Value(vnode->belief()).value;
	vnode->lower_bound(value);
}

void AEMS::InitUpperBound(VNode* vnode, BeliefUpperBound* upper_bound,
	History& history) {
	double value = upper_bound->Value(vnode->belief());
	vnode->upper_bound(value);
}

void AEMS::Expand(VNode* vnode, BeliefLowerBound* lower_bound,
	BeliefUpperBound* upper_bound, const BeliefMDP* model, History& history) {
	vector<QNode*>& children = vnode->children();
	logd << "- Expanding vnode " << vnode << endl;
	for (int action = 0; action < model->NumActions(); action++) {
		logd << " Action " << action << endl;
		QNode* qnode = new QNode(vnode, action);
		children.push_back(qnode);

		Expand(qnode, lower_bound, upper_bound, model, history);
	}
	logd << "* Expansion complete!" << endl;
}

void AEMS::Expand(QNode* qnode, BeliefLowerBound* lb, BeliefUpperBound* ub,
	const BeliefMDP* model, History& history) {
	VNode* parent = qnode->parent();
	int action = qnode->edge();
	map<OBS_TYPE, VNode*>& children = qnode->children();

	const Belief* belief = parent->belief();
	// cout << *belief << endl;

	double step_reward = model->StepReward(belief, qnode->edge());

	map<OBS_TYPE, double> obss;
	model->Observe(belief, qnode->edge(), obss);

	double lower_bound = step_reward;
	double upper_bound = step_reward;

	// Create new belief nodes
	for (map<OBS_TYPE, double>::iterator it = obss.begin(); it != obss.end(); it++) {
		OBS_TYPE obs = it->first;
		double weight = it->second;
		logd << "[AEMS::Expand] Creating node for obs " << obs
			<< " with weight " << weight << endl;
		VNode* vnode = new VNode(model->Tau(belief, action, obs),
			parent->depth() + 1, qnode, obs);
		vnode->likelihood = weight;
		logd << " New node created!" << endl;
		children[obs] = vnode;

		InitLowerBound(vnode, lb, history);
		InitUpperBound(vnode, ub, history);

		lower_bound += weight * Globals::Discount() * vnode->lower_bound();
		upper_bound += weight * Globals::Discount() * vnode->upper_bound();
	}

	qnode->step_reward = step_reward;
	qnode->lower_bound(lower_bound);
	qnode->upper_bound(upper_bound);
}

void AEMS::Update(int action, OBS_TYPE obs) {
	logi << "- Updating belief, history and root with action " << action
		<< " and observation " << obs << "...";

	if (reuse_) {
		VNode* node = root_->Child(action)->Child(obs);
		root_->Child(action)->children().erase(obs);
		delete root_;
		root_ = node;
		root_->likelihood = 1.0;
		root_->parent(NULL);

		belief_ = root_->belief();
	} else {
		delete root_;
		root_ = NULL;

		Belief* new_belief = model_->Tau(belief_, action, obs);
		delete belief_;
		belief_ = new_belief;

		// belief_->Update(action, obs); // approx update
	}

	history_.Add(action, obs);

	logi << "Done!" << endl;
}

void AEMS::belief(Belief* b) {
	belief_ = b;
	history_.Truncate(0);
	delete root_;
	root_ = NULL;
}

} // namespace despot
