#include <despot/util/util.h>

using namespace std;

namespace despot {

string repeat(string str, int n) {
	ostringstream oss;
	for (int i = 0; i < n; i++)
		oss << str;
	return oss.str();
}

double erf(double x) {
	// constants
	double a1 = 0.254829592;
	double a2 = -0.284496736;
	double a3 = 1.421413741;
	double a4 = -1.453152027;
	double a5 = 1.061405429;
	double p = 0.3275911;
	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = fabs(x);
	// A&S formula 7.1.26
	double t = 1.0 / (1.0 + p * x);
	double y = 1.0
		- (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

	return sign * y;
}

// CDF of the normal distribution
double gausscdf(double x, double mean, double sigma) {
	return 0.5 * (1 + erf((x - mean) / (sqrt(2) * sigma)));
}

vector<string> Tokenize(string line, char delim) {
	// Tokenizes a string on a delim and strips whitespace from the tokens
	vector<string> tokens;
	stringstream ss(line);
	string item;
	while (getline(ss, item, delim)) {
		while (isspace(item[0]))
			item.erase(item.begin());
		while (isspace(item[item.length()-1]))
			item.erase(item.end() - 1);
		tokens.push_back(item);
	}
	return tokens;
}

vector<string> Tokenize(const string& str, const string& delimiters) {
	vector<string> tokens;
	string::size_type last_pos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, last_pos);

	while (string::npos != pos || string::npos != last_pos) {
		tokens.push_back(str.substr(last_pos, pos - last_pos));

		last_pos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, last_pos);
	}
	return tokens;
}

} // namespace despot

