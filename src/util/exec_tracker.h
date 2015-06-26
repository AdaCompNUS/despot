#ifndef EXECTRACKER_H
#define EXECTRACKER_H

#include <iostream>
#include <string>
#include <map>
#include <set>

using namespace std;

class ExecTracker {
private:
	map<string, string> creation_loc_;
public:
	ExecTracker();

	void Track(string addr, string position);
	void Untrack(string addr);
	void Print(ostream& out = cout) const;
	void PrintLocs(ostream& out = cout) const;
};

#endif
