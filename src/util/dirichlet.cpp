#include <despot/util/dirichlet.h>

using namespace std;

namespace despot {

Dirichlet::Dirichlet(vector<double> alpha) {
	alpha_ = alpha;
}

vector<double> Dirichlet::alpha() {
	return alpha_;
}

vector<double> Dirichlet::Next() {
	return Next(alpha_);
}

vector<double> Dirichlet::Next(vector<double> alpha) {
	int dim = alpha.size();
	vector<double> x(dim, 0);
	double sum = 0;
	for (int i = 0; i < dim; i++) {
		x[i] = Gamma::Next(alpha[i], 1);
		sum = sum + x[i];
	}
	for (int i = 0; i < dim; i++)
		x[i] = x[i] / sum;
	return x;
}

} // namespace despot
