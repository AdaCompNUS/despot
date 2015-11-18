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
#include <sys/time.h>

namespace despot {

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
std::string to_string(T t) {
  std::ostringstream oss;
	oss << t;
	return oss.str();
}

template<typename T>
bool operator<(const std::vector<T>& first, const std::vector<T>& second) {
	for (int i = 0; i < first.size(); i++) {
		if (first[i] != second[i]) {
			return first[i] < second[i];
		}
	}

	return 0;
}

template<typename K, typename V>
bool CompareFirst(std::pair<K, V> p1, std::pair<K, V> p2) {
	return p1.first > p2.first;
}

template<typename K, typename V>
bool CompareSecond(std::pair<K, V> p1, std::pair<K, V> p2) {
	return p1.second > p2.second;
}

template<typename K, typename V>
std::vector<std::pair<K, V> > SortByValue(std::map<K, V> m) {
	std::vector<std::pair<K, V> > v;
	for (typename std::map<K, V>::iterator it = m.begin(); it != m.end();
		it++) {
		v.push_back(std::pair<K, V>(it->first, it->second));
	}
	std::sort(v.begin(), v.end(), CompareSecond<K, V>);
	return v;
}

template<typename K, typename V>
std::vector<std::pair<K, V> > SortByKey(std::map<K, V> m) {
	std::vector<std::pair<K, V> > v;
	for (typename std::map<K, V>::iterator it = m.begin(); it != m.end();
		it++) {
		v.push_back(std::pair<K, V>(it->first, it->second));
	}
	std::sort(v.begin(), v.end(), CompareFirst<K, V>);
	return v;
}

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

inline std::string lower(std::string str) {
  std::locale loc;
	std::string copy = str;
	for (int i = 0; i < copy.length(); i++)
		copy[i] = std::tolower(copy[i], loc);
	return copy;
}

std::string repeat(std::string str, int n);

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

std::vector<std::string> Tokenize(std::string line, char delim);
std::vector<std::string> Tokenize(const std::string& str, const std::string& delimiters = " ");

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, std::pair<K, V> p) {
	os << "(" << p.first << ", " << p.second << ")";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> vec) {
	os << "[";
	for (int i = 0; i < vec.size(); i++)
		os << (i == 0 ? "" : ", ") << vec[i];
	os << "]";
	return os;
}

template<typename T>
void SetSize(std::vector<std::vector<T> > v, int d1, int d2) {
	v.resize(d1);
	for (int i = 0; i < d1; i++)
		v[i].resize(d2);
}

template<typename T>
void SetSize(std::vector<std::vector<std::vector<T> > >& v, int d1, int d2, int d3) {
	v.resize(d1);
	for(int i=0; i<d1; i++) {
		v[i].resize(d2);
		for(int j=0; j<d2; j++)
		v[i][j].resize(d3);
	}
}

template<typename K, typename V>
std::vector<K>* GetKeys(const std::map<K, V> m) {
	std::vector<K>* k = new std::vector<K>();
	for (typename std::map<K, V>::iterator it = m.begin(); it != m.end(); it++)
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

} // namespace despot

#endif
