#include <despot/core/node.h>
#include <despot/solver/despot.h>

using namespace std;

namespace despot {

/* =============================================================================
 * VNode class
 * =============================================================================*/

VNode::VNode(vector<State*>& particles, int depth, QNode* parent,
	OBS_TYPE edge) :
	particles_(particles),
	belief_(NULL),
	depth_(depth),
	parent_(parent),
	edge_(edge),
	vstar(this),
	likelihood(1) {
	logd << "Constructed vnode with " << particles_.size() << " particles"
		<< endl;
	for (int i = 0; i < particles_.size(); i++) {
		logd << " " << i << " = " << *particles_[i] << endl;
	}
}

VNode::VNode(Belief* belief, int depth, QNode* parent, OBS_TYPE edge) :
	belief_(belief),
	depth_(depth),
	parent_(parent),
	edge_(edge),
	vstar(this),
	likelihood(1) {
}

VNode::VNode(int count, double value, int depth, QNode* parent, OBS_TYPE edge) :
	belief_(NULL),
	depth_(depth),
	parent_(parent),
	edge_(edge),
	count_(count),
	value_(value) {
}

VNode::~VNode() {
	for (int a = 0; a < children_.size(); a++) {
		QNode* child = children_[a];
		assert(child != NULL);
		delete child;
	}
	children_.clear();

	if (belief_ != NULL)
		delete belief_;
}

Belief* VNode::belief() const {
	return belief_;
}

const vector<State*>& VNode::particles() const {
	return particles_;
}

void VNode::depth(int d) {
	depth_ = d;
}

int VNode::depth() const {
	return depth_;
}

void VNode::parent(QNode* parent) {
	parent_ = parent;
}

QNode* VNode::parent() {
	return parent_;
}

OBS_TYPE VNode::edge() {
	return edge_;
}

double VNode::Weight() const {
	return State::Weight(particles_);
}

const vector<QNode*>& VNode::children() const {
	return children_;
}

vector<QNode*>& VNode::children() {
	return children_;
}

const QNode* VNode::Child(int action) const {
	return children_[action];
}

QNode* VNode::Child(int action) {
	return children_[action];
}

int VNode::Size() const {
	int size = 1;
	for (int a = 0; a < children_.size(); a++) {
		size += children_[a]->Size();
	}
	return size;
}

int VNode::PolicyTreeSize() const {
	if (children_.size() == 0)
		return 0;

	QNode* best = NULL;
	for (int a = 0; a < children_.size(); a++) {
		QNode* child = children_[a];
		if (best == NULL || child->lower_bound() > best->lower_bound())
			best = child;
	}
	return best->PolicyTreeSize();
}

void VNode::default_move(ValuedAction move) {
	default_move_ = move;
}

ValuedAction VNode::default_move() const {
	return default_move_;
}

void VNode::lower_bound(double value) {
	lower_bound_ = value;
}

double VNode::lower_bound() const {
	return lower_bound_;
}

void VNode::upper_bound(double value) {
	upper_bound_ = value;
}

double VNode::upper_bound() const {
	return upper_bound_;
}

bool VNode::IsLeaf() {
	return children_.size() == 0;
}

void VNode::Add(double val) {
	value_ = (value_ * count_ + val) / (count_ + 1);
	count_++;
}

void VNode::count(int c) {
	count_ = c;
}
int VNode::count() const {
	return count_;
}
void VNode::value(double v) {
	value_ = v;
}
double VNode::value() const {
	return value_;
}

void VNode::Free(const DSPOMDP& model) {
	for (int i = 0; i < particles_.size(); i++) {
		model.Free(particles_[i]);
	}

	for (int a = 0; a < children().size(); a++) {
		QNode* qnode = Child(a);
		map<OBS_TYPE, VNode*>& children = qnode->children();
		for (map<OBS_TYPE, VNode*>::iterator it = children.begin();
			it != children.end(); it++) {
			it->second->Free(model);
		}
	}
}

void VNode::PrintPolicyTree(int depth, ostream& os) {
	if (depth != -1 && this->depth() > depth)
		return;

	vector<QNode*>& qnodes = children();
	if (qnodes.size() == 0) {
		int astar = this->default_move().action;
		os << this << "-a=" << astar << endl;
	} else {
		QNode* qstar = NULL;
		for (int a = 0; a < qnodes.size(); a++) {
			QNode* qnode = qnodes[a];
			if (qstar == NULL || qnode->lower_bound() > qstar->lower_bound()) {
				qstar = qnode;
			}
		}

		os << this << "-a=" << qstar->edge() << endl;

		vector<OBS_TYPE> labels;
		map<OBS_TYPE, VNode*>& vnodes = qstar->children();
		for (map<OBS_TYPE, VNode*>::iterator it = vnodes.begin();
			it != vnodes.end(); it++) {
			labels.push_back(it->first);
		}

		for (int i = 0; i < labels.size(); i++) {
			if (depth == -1 || this->depth() + 1 <= depth) {
				os << repeat("|   ", this->depth()) << "| o=" << labels[i]
					<< ": ";
				qstar->Child(labels[i])->PrintPolicyTree(depth, os);
			}
		}
	}
}

void VNode::PrintTree(int depth, ostream& os) {
	if (depth != -1 && this->depth() > depth)
		return;

	if (this->depth() == 0) {
		os << "d - default value" << endl
			<< "l - lower bound" << endl
			<< "u - upper bound" << endl
			<< "r - totol weighted one step reward" << endl
			<< "w - total particle weight" << endl;
	}

	os << "(" << "d:" << this->default_move().value <<
		" l:" << this->lower_bound() << ", u:" << this->upper_bound()
		<< ", w:" << this->Weight() << ", weu:" << DESPOT::WEU(this)
		<< ")"
		<< endl;


	vector<QNode*>& qnodes = children();
	for (int a = 0; a < qnodes.size(); a++) {
		QNode* qnode = qnodes[a];

		vector<OBS_TYPE> labels;
		map<OBS_TYPE, VNode*>& vnodes = qnode->children();
		for (map<OBS_TYPE, VNode*>::iterator it = vnodes.begin();
			it != vnodes.end(); it++) {
			labels.push_back(it->first);
		}

		os << repeat("|   ", this->depth()) << "a="
			<< qnode->edge() << ": "
			<< "(d:" << qnode->default_value << ", l:" << qnode->lower_bound()
			<< ", u:" << qnode->upper_bound()
			<< ", r:" << qnode->step_reward << ")" << endl;

		for (int i = 0; i < labels.size(); i++) {
			if (depth == -1 || this->depth() + 1 <= depth) {
				os << repeat("|   ", this->depth()) << "| o=" << labels[i]
					<< ": ";
				qnode->Child(labels[i])->PrintTree(depth, os);
			}
		}
	}
}

/* =============================================================================
 * QNode class
 * =============================================================================*/

QNode::QNode(VNode* parent, int edge) :
	parent_(parent),
	edge_(edge),
	vstar(NULL) {
}

QNode::QNode(int count, double value) :
	count_(count),
	value_(value) {
}

QNode::~QNode() {
	for (map<OBS_TYPE, VNode*>::iterator it = children_.begin();
		it != children_.end(); it++) {
		assert(it->second != NULL);
		delete it->second;
	}
	children_.clear();
}

void QNode::parent(VNode* parent) {
	parent_ = parent;
}

VNode* QNode::parent() {
	return parent_;
}

int QNode::edge() {
	return edge_;
}

map<OBS_TYPE, VNode*>& QNode::children() {
	return children_;
}

VNode* QNode::Child(OBS_TYPE obs) {
	return children_[obs];
}

int QNode::Size() const {
	int size = 0;
	for (map<OBS_TYPE, VNode*>::const_iterator it = children_.begin();
		it != children_.end(); it++) {
		size += it->second->Size();
	}
	return size;
}

int QNode::PolicyTreeSize() const {
	int size = 0;
	for (map<OBS_TYPE, VNode*>::const_iterator it = children_.begin();
		it != children_.end(); it++) {
		size += it->second->PolicyTreeSize();
	}
	return 1 + size;
}

double QNode::Weight() const {
	double weight = 0;
	for (map<OBS_TYPE, VNode*>::const_iterator it = children_.begin();
		it != children_.end(); it++) {
		weight += it->second->Weight();
	}
	return weight;
}

void QNode::lower_bound(double value) {
	lower_bound_ = value;
}

double QNode::lower_bound() const {
	return lower_bound_;
}

void QNode::upper_bound(double value) {
	upper_bound_ = value;
}

double QNode::upper_bound() const {
	return upper_bound_;
}

void QNode::Add(double val) {
	value_ = (value_ * count_ + val) / (count_ + 1);
	count_++;
}

void QNode::count(int c) {
	count_ = c;
}

int QNode::count() const {
	return count_;
}

void QNode::value(double v) {
	value_ = v;
}

double QNode::value() const {
	return value_;
}

} // namespace despot
