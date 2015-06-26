#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
// #include <chrono>
#include <locale>

using namespace std;
// using namespace chrono;

/*
// NOTE: disabled C++11 feature
template<typename T>
void write(ostringstream& os, T t) {
	os << t;
}

template<typename T, typename ... Args>
void write(ostringstream& os, T t, Args ... args) {
	os << t;
	write(os, args...);
}

template<typename T, typename ... Args>
string concat(T t, Args ... args) {
	ostringstream os;
	write(os, t, args...);
	return os.str();
}

template<typename T>
string concat(vector<T> v) {
	ostringstream os;
	for (int i = 0; i < v.size(); i++) {
		os << v[i];
		os << " ";
	}
	return os.str();
}
*/

template<typename T>
string to_string(T t) {
	ostringstream oss;
	oss << t;
	return oss.str();
}

template<typename T>
bool operator<(const vector<T>& first, const vector<T>& second) {
	for (int i = 0; i < first.size(); i++) {
		if (first[i] != second[i]) {
			return first[i] < second[i];
		}
	}

	return 0;
}

template<typename K, typename V>
bool CompareFirst(pair<K, V> p1, pair<K, V> p2) {
	return p1.first > p2.first;
}

template<typename K, typename V>
bool CompareSecond(pair<K, V> p1, pair<K, V> p2) {
	return p1.second > p2.second;
}

template<typename K, typename V>
vector<pair<K, V> > SortByValue(map<K, V> m) {
	vector<pair<K, V> > v;
	for (typename map<K, V>::iterator it = m.begin(); it != m.end();
		it++) {
		v.push_back(pair<K, V>(it->first, it->second));
	}
	sort(v.begin(), v.end(), CompareSecond<K, V>);
	return v;
}

template<typename K, typename V>
vector<pair<K, V> > SortByKey(map<K, V> m) {
	vector<pair<K, V> > v;
	for (typename map<K, V>::iterator it = m.begin(); it != m.end();
		it++) {
		v.push_back(pair<K, V>(it->first, it->second));
	}
	sort(v.begin(), v.end(), CompareFirst<K, V>);
	return v;
}

#include <sys/time.h>
inline double get_time_second() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

/*
// NOTE: disabled C++11 feature
inline double get_time_second() {
	system_clock::time_point tp = system_clock::now();
	system_clock::duration dtn = tp.time_since_epoch();
	return ((double) dtn.count()) * system_clock::period::num
		/ system_clock::period::den;
}
*/

inline string lower(string str) {
	locale loc;
	string copy = str;
	for (int i = 0; i < copy.length(); i++)
		copy[i] = tolower(copy[i], loc);
	return copy;
}

string repeat(string str, int n);

double erf(double x);
double gausscdf(double x, double mean, double sigma);

inline bool CheckFlag(int flags, int bit) {
	return (flags & (1 << bit)) != 0;
}
inline void SetFlag(int& flags, int bit) {
	flags = (flags | (1 << bit));
}
inline void UnsetFlag(int& flags, int bit) {
	flags = flags & ~(1 << bit);
}

vector<string> Tokenize(string line, char delim);
vector<string> Tokenize(const string& str, const string& delimiters = " ");

template<typename K, typename V>
ostream& operator<<(ostream& os, pair<K, V> p) {
	os << "(" << p.first << ", " << p.second << ")";
	return os;
}

template<typename T>
ostream& operator<<(ostream& os, vector<T> vec) {
	os << "[";
	for (int i = 0; i < vec.size(); i++)
		os << (i == 0 ? "" : ", ") << vec[i];
	os << "]";
	return os;
}

template<typename T>
void SetSize(vector<vector<T> > v, int d1, int d2) {
	v.resize(d1);
	for (int i = 0; i < d1; i++)
		v[i].resize(d2);
}

template<typename T>
void SetSize(vector<vector<vector<T> > >& v, int d1, int d2, int d3) {
	v.resize(d1);
	for(int i=0; i<d1; i++) {
		v[i].resize(d2);
		for(int j=0; j<d2; j++)
		v[i][j].resize(d3);
	}
}

template<typename K, typename V>
vector<K>* GetKeys(const map<K, V> m) {
	vector<K>* k = new vector<K>();
	for (typename map<K, V>::iterator it = m.begin(); it != m.end(); it++)
		k->push_back(it->first);
	return k;
}

template <class DstType, class SrcType>
bool IsType(const SrcType* src) {
	  return dynamic_cast<const DstType*>(src) != 0;
}

/*
// NOTE: disabled C++11 feature
// Functions for hashing data structs
namespace std {
template<class T>
inline void hash_combine(size_t& seed, const T& v) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename S, typename T>
struct hash<pair<S, T> > {
	inline size_t operator()(const pair<S, T>& v) const {
		size_t seed = 0;
		::hash_combine(seed, v.first);
		::hash_combine(seed, v.second);
		return seed;
	}
};

template<typename T>
struct hash<vector<T> > {
	inline size_t operator()(const vector<T>& v) const {
		size_t seed = 0;
		for (int i = 0; i < v.size(); i++) {
			::hash_combine(seed, v[i]);
		}
		return seed;
	}
};
}
*/
#endif
