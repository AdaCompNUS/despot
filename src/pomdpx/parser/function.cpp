#include <despot/pomdpx/parser/function.h>
#include <stdexcept>

using namespace std;

namespace despot {

/* =============================================================================
 * Function class
 * =============================================================================*/

Function::Function() {
}

Function::Function(NamedVar* child, vector<NamedVar*> parents) :
	child_(child),
	parents_(parents) {
	int parent_size = 1;
	for (int i = 0; i < parents.size(); i++)
		parent_size *= parents[i]->values().size();

	if (child->values().size() < 100) { // TODO: possible refactoring to use a single data structure
		values_.resize(parent_size);
		for (int i = 0; i < parent_size; i++)
			values_[i].resize(child->values().size());
	} else {
		map_.resize(parent_size);
	}
	// cout << "size " << values_.size() << " " << values_[0].size() << endl;
}

Function::~Function() {
}

bool Function::SetValue(vector<string>& keys, int key_pos, int pid, int cid,
	const vector<double>& values, int start, int end) {
	if (key_pos == parents_.size() + 1) {
		assert(start == end - 1);

		if (values_.size() > 0) // TODO: possible refactoring to use a single data structure
			values_[pid][cid] = values[start];
		else {
			if (values[start] != 0.0) {
				map_[pid][cid] = values[start];
			} else {
				map_[pid].erase(cid);
			}
		}

		return true;
	} else {
		NamedVar* curr =
			(key_pos == keys.size() - 1) ? child_ : parents_[key_pos];
		const vector<string>& dom = curr->values();

		if (keys[key_pos] == "*") {
			for (int i = 0; i < dom.size(); i++) {
				int next_cid = cid, next_pid = pid;
				if (key_pos == keys.size() - 1)
					next_cid = i;
				else
					next_pid = pid * dom.size() + i;

				if (!SetValue(keys, key_pos + 1, next_pid, next_cid, values,
					start, end))
					return false;
			}
			return true;
		} else if (keys[key_pos] == "-") {
			int block_size = (end - start) / curr->values().size();

			for (int i = 0; i < dom.size(); i++) {
				int next_cid = cid, next_pid = pid;
				if (key_pos == keys.size() - 1)
					next_cid = i;
				else
					next_pid = pid * dom.size() + i;

				if (!SetValue(keys, key_pos + 1, next_pid, next_cid, values,
					start + i * block_size, start + (i + 1) * block_size))
					return false;
			}
			return true;
		} else {
			int id = curr->IndexOf(keys[key_pos]);
			if (id < 0) {
				cerr << "ERROR: " << keys[key_pos]
					<< " is not in the domain of " << curr->name() << endl;
				return false;
			}

			if (key_pos == keys.size() - 1)
				cid = id;
			else
				pid = pid * dom.size() + id;

			return SetValue(keys, key_pos + 1, pid, cid, values, start, end);
		}
	}
}

bool Function::SetValue(const vector<string>& keys,
	const vector<double>& values) {
	vector<string> copy = keys;
	return SetValue(copy, 0, 0, 0, values, 0, values.size());
}

void Function::SetValue(int pid, int cid, double value) {
	if (values_.size() > 0) { // TODO: possible refactoring to use a single data structure
		values_[pid][cid] = value;
	} else {
		map_[pid][cid] = value;
	}
}

double Function::GetValue(int pid, int cid) const {
	if (values_.size() > 0) { // TODO: possible refactoring to use a single data structure
		return values_[pid][cid];
	} else {
		map<int, double>::const_iterator it = map_[pid].find(cid);
		return it != map_[pid].end() ? it->second : 0;
	}
}

double Function::GetValue(int cid) const {
	// return values_[Variable::ComputeCurrentIndex(parents_)][cid];
	return GetValue(Variable::ComputeCurrentIndex(parents_), cid);
}

const vector<NamedVar*>& Function::parents() const {
	return parents_;
}

int Function::ParentSize() const {
	if (values_.size() > 0) // TODO: possible refactoring to use a single data structure
		return values_.size();
	else
		return map_.size();
}

int Function::ChildSize() const {
	return child_->values().size();
}

const NamedVar* Function::child() const {
	return child_;
}

double Function::ComputeConstrainedMaximum(const NamedVar* var,
	int value) const {
	if (var->values().size() <= value)
		throw std::invalid_argument(
			"Variable " + var->name() + " cannot have value " + to_string(value)
				+ ".");

	// cout << "Computing constrained maximum (" << var->name() << " = " << value << ")" << endl;
	// cout << *this << endl;

	int parent_size = ParentSize();
	int child_size = ChildSize();
	double max = Globals::NEG_INFTY;
	if (var == child_) {
		for (int p = 0; p < parent_size; p++)
			if (GetValue(p, value) > max)
				max = GetValue(p, value);
	} else {
		int id = -1;
		for (int i = 0; i < parents_.size(); i++)
			if (parents_[i] == var) {
				id = i;
				break;
			}

		for (int p = 0; p < parent_size; p++) {
			vector<int> vec = Variable::ComputeIndexVec(parents_, p);
			if (id == -1 || vec[id] == value) {
				for (int c = 0; c < child_size; c++) {
					if (GetValue(p, c) > max) {
						max = GetValue(p, c);
					}
				}
			}
		}
	}

	// cout << "max = " << max << endl;

	return max;
}

double Function::ComputeConstrainedMinimum(const NamedVar* var,
	int value) const {
	if (var->values().size() <= value)
		throw std::invalid_argument(
			"Variable " + var->name() + " cannot have value " + to_string(value)
				+ ".");

	// cout << "Computing constrained minimum (" << var->name() << " = " << value << ")" << endl;
	// cout << *this << endl;
	int parent_size = ParentSize();
	int child_size = ChildSize();
	double min = Globals::POS_INFTY;
	if (var == child_) {
		for (int p = 0; p < parent_size; p++)
			if (GetValue(p, value) < min)
				min = GetValue(p, value);
	} else {
		int id = -1;
		for (int i = 0; i < parent_size; i++)
			if (parents_[i] == var) {
				id = i;
				break;
			}

		for (int p = 0; p < parent_size; p++) {
			vector<int> vec = Variable::ComputeIndexVec(parents_, p);
			if (id == -1 || vec[id] == value) {
				for (int c = 0; c < child_size; c++) {
					if (GetValue(p, c) < min) {
						min = GetValue(p, c);
					}
				}
			}
		}
	}
	// cout << "min = " << min << endl;

	return min;
}

ostream& operator<<(ostream& os, const Function& func) {
	os << "Vars: [";
	for (int i = 0; i < func.parents_.size(); i++) {
		os << func.parents_[i]->name()
			<< (i == func.parents_.size() - 1 ? " -> " : ", ");
	}
	os << func.child_->name() << "]" << endl; // TODO: should print curr_name here but prev_name above for state variable

	if (func.values_.size() > 0) {
		for (int p = 0; p < func.values_.size(); p++) {
			vector<int> index = Variable::ComputeIndexVec(func.parents_, p);
			vector<string> str;
			for (int i = 0; i < index.size(); i++)
				str.push_back(func.parents_[i]->GetValue(index[i]));
			os << " " << str << ":";
			for (int c = 0; c < func.values_[p].size(); c++) {
				if (func.values_[p][c] != 0)
					os << " " << func.child_->GetValue(c) << ":"
						<< func.values_[p][c];
			}
			if (p != func.values_.size() - 1)
				os << endl;
		}
	} else {
		for (int p = 0; p < func.map_.size(); p++) {
			vector<int> index = Variable::ComputeIndexVec(func.parents_, p);
			vector<string> str;
			for (int i = 0; i < index.size(); i++)
				str.push_back(func.parents_[i]->GetValue(index[i]));
			os << " " << str << ":";

			for (map<int, double>::const_iterator it = func.map_[p].begin();
				it != func.map_[p].end(); it++) {
				os << " " << func.child_->GetValue(it->first) << ":"
					<< it->second;
			}
			if (p != func.map_.size() - 1)
				os << endl;
		}
	}
	return os;
}

/* =============================================================================
 * CPT class
 * =============================================================================*/

CPT::~CPT() {
}

/* =============================================================================
 * TabularCPT class
 * =============================================================================*/

TabularCPT::TabularCPT(NamedVar* child, vector<NamedVar*> parents) {
	child_ = child;
	parents_ = parents;

	int parent_size = 1;
	for (int i = 0; i < parents.size(); i++)
		parent_size *= parents[i]->values().size();

	if (child->values().size() < 100) { // TODO: possible refactoring to use a single data structure
		values_.resize(parent_size);
		for (int i = 0; i < parent_size; i++)
			values_[i].resize(child->values().size());
	} else {
		map_.resize(parent_size);
	}
}

bool TabularCPT::Validate() const {
	int parent_size = ParentSize(), child_size = ChildSize();

	for (int p = 0; p < parent_size; p++) {
		double sum = 0;
		for (int c = 0; c < child_size; c++) {
			sum += GetValue(p, c);
		}

		if (fabs(sum - 1.0) > 1e-6) {
			vector<int> index = Variable::ComputeIndexVec(parents_, p);
			cerr << "ERROR: Probabilities do not sum to one when parents = [";
			for (int i = 0; i < index.size(); i++) {
				cerr << (i == 0 ? "" : ", ") << parents_[i]->name() << "="
					<< parents_[i]->GetValue(index[i]);
			}
			cerr << "]" << endl;

			cerr << " Sum = " << sum << "; Entries: [";
			for (int i = 0; i < child_size; i++) {
				cerr << (i == 0 ? "" : ", ") << child_->GetValue(i) << "="
					<< GetValue(p, i);
			}
			cerr << "]" << endl;

			return false;
		}
	}

	return true;
}

/* =============================================================================
 * IMPORTANT: deal with limited numerical precision
 * Numbers are often represented approximately as finite precision binary
 * numbers, rather than exactly. For example, 0.1 can only be exactly
 * represented by an infinitely long binary number. As a result, when
 * ComputeIndex is called multiple times on uniform binary distribution, the
 * sum argument always converges to 1 within a constant number of steps, and
 * thus the generated index distribution will be wrong after that. The
 * implementation here uses inexact division to alleviate this problem.
 * =============================================================================*/
int TabularCPT::ComputeIndex(int pid, double& sum) const {
	if (sparse_values_.size() != 0) {
		for (int i = 0; i < sparse_values_[pid].size(); i++) {
			const pair<int, double>& pair = sparse_values_[pid][i];
			if (sum < pair.second) {
				sum /= pair.second * (1 + 1E-9);
				return pair.first;
			}
			sum -= pair.second;
		}
		return -1;
	} else {
		for (int cur = 0; cur < values_[pid].size(); cur++) {
			if (sum < values_[pid][cur]) {
				sum /= values_[pid][cur] * (1 + 1E-9);
				return cur;
			}
			sum -= values_[pid][cur];
		}
	}

	assert(false);
	return -1;
}

int TabularCPT::ComputeCurrentIndex(double& sum) const {
	return ComputeIndex(Variable::ComputeCurrentIndex(parents_), sum);
}

void TabularCPT::ComputeSparseChildDistribution() {
	int parent_size = ParentSize(), child_size = ChildSize();
	sparse_values_.resize(parent_size);
	for (int p = 0; p < parent_size; p++) {
		for (int c = 0; c < child_size; c++) {
			if (GetValue(p, c) > 0)
				sparse_values_[p].push_back(
					pair<int, double>(c, GetValue(p, c)));
		}
	}
}

bool TabularCPT::IsIdentityUnderConstraint(const NamedVar* var,
	int value) const {
	if (var->values().size() <= value)
		throw std::invalid_argument(
			"Variable " + var->name() + " cannot have value " + to_string(value)
				+ ".");

	int child_id = -1;
	for (int i = 0; i < parents_.size(); i++)
		if (parents_[i] == child_)
			child_id = i;

	if (child_id == -1)
		return false;

	int var_id = -1;
	for (int i = 0; i < parents_.size(); i++)
		if (parents_[i] == var) {
			var_id = i;
			break;
		}

	int parent_size = ParentSize();
	for (int p = 0; p < parent_size; p++) {
		vector<int> vec = Variable::ComputeIndexVec(parents_, p);
		if (var_id == -1 || vec[var_id] == value) {
			if (GetValue(p, vec[child_id]) != 1.0)
				return false;
		}
	}

	return true;
}

CPT* TabularCPT::CreateNoisyVariant(double noise) const {
	int parent_size = ParentSize(), child_size = ChildSize();

	TabularCPT* cpt = new TabularCPT(child_, parents_);
	cpt->map_.clear();

	cpt->values_.resize(parent_size);
	for (int i = 0; i < parent_size; i++)
		cpt->values_[i].resize(child_size);

	for (int p = 0; p < parent_size; p++) {
		for (int c = 0; c < child_size; c++) {
			cpt->values_[p][c] = (GetValue(p, c) + noise / child_size)
				/ (1 + noise);
		}
	}

	return cpt;
}

/* =============================================================================
 * HierarchyCPT class
 * =============================================================================*/

HierarchyCPT::HierarchyCPT(NamedVar* child, vector<NamedVar*> parents) {
	child_ = child;
	parents_ = parents;
	cpts_.resize(parents_[0]->Size());
}

HierarchyCPT::~HierarchyCPT() {
}

void HierarchyCPT::SetParents(int val, vector<NamedVar*> parents) {
	cpts_[val] = new TabularCPT(child_, parents);
}

bool HierarchyCPT::SetValue(int val, const vector<string>& keys,
	const vector<double>& values) {
	return cpts_[val]->SetValue(keys, values);
}

double HierarchyCPT::GetValue(int cid) const {
	int val = parents_[0]->curr_value;
	return cpts_[val]->GetValue(
		Variable::ComputeCurrentIndex(cpts_[val]->parents_), cid);
}

int HierarchyCPT::ComputeCurrentIndex(double& sum) const {
	return cpts_[parents_[0]->curr_value]->ComputeCurrentIndex(sum);
}

int HierarchyCPT::ParentSize() const {
	int size = 1;
	for (int i = 0; i < parents_.size(); i++)
		size *= parents_[i]->Size();
	return size;
}

bool HierarchyCPT::Validate() const {
	for (int i = 0; i < cpts_.size(); i++)
		if (!cpts_[i]->Validate())
			return false;
	return true;
}

void HierarchyCPT::ComputeSparseChildDistribution() {
	for (int i = 0; i < cpts_.size(); i++)
		cpts_[i]->ComputeSparseChildDistribution();
}

bool HierarchyCPT::IsIdentityUnderConstraint(const NamedVar* var,
	int value) const {
	if (var == parents_[0])
		return cpts_[value]->IsIdentityUnderConstraint(var, value);

	for (int i = 0; i < cpts_.size(); i++)
		if (!cpts_[i]->IsIdentityUnderConstraint(var, value))
			return false;
	return true;
}

CPT* HierarchyCPT::CreateNoisyVariant(double noise) const {
	HierarchyCPT* hcpt = new HierarchyCPT(child_, parents_);
	for (int i = 0; i < cpts_.size(); i++)
		hcpt->cpts_[i] = static_cast<TabularCPT*>(cpts_[i]->CreateNoisyVariant(
			noise));
	return hcpt;
}

ostream& operator<<(std::ostream& os, const HierarchyCPT& hcpt) {
	os << "Root variable: " << hcpt.parents_[0]->name() << endl;
	for (int i = 0; i < hcpt.cpts_.size(); i++)
		os << hcpt.parents_[0]->name() << " = " << hcpt.parents_[0]->GetValue(i)
			<< " " << *(hcpt.cpts_[i]) << endl;
	return os;
}

} // namespace despot
