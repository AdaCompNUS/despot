#ifndef EXECTRACKER_H
#define EXECTRACKER_H

#include <iostream>
#include <string>
#include <map>
#include <set>

namespace despot {

class ExecTracker {
private:
  std::map<std::string, std::string> creation_loc_;
public:
	ExecTracker();

	void Track(std::string addr, std::string position);
	void Untrack(std::string addr);
	void Print(std::ostream& out = std::cout) const;
	void PrintLocs(std::ostream& out = std::cout) const;
};

} // namespace despot

#endif
