#ifndef GLOBALS_H
#define GLOBALS_H

#include <typeinfo>
#include <memory>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <ctime>
#include <vector>
#include <queue>
#include <cmath>
#include <cassert>
#include <limits>
#include <sstream>
#include <map>
#include <inttypes.h>
#include "config.h"

#include "util/exec_tracker.h"
#include "util/logging.h"

using namespace std;

typedef uint64_t OBS_TYPE;

namespace Globals {
extern const double NEG_INFTY;
extern const double POS_INFTY;
extern const double INF;
extern const double TINY;

extern Config config;
extern ExecTracker tracker;

inline bool Fequals(double a, double b) {
	return fabs(a - b) < TINY;
}

inline double Discount() {
	return config.discount;
}

inline double Discount(int d) {
	return pow(config.discount, d);
}

inline void Track(string addr, string loc) {
	tracker.Track(addr, loc);
}

inline void Untrack(string addr) {
	tracker.Untrack(addr);
}

inline void PrintLocs() {
	tracker.PrintLocs();
}
} // namespace

#endif
