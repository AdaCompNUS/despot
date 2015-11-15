#include <despot/util/exec_tracker.h>

using namespace std;

namespace despot {

ExecTracker::ExecTracker() {
}

void ExecTracker::Track(string addr, string position) {
	creation_loc_[addr] = position;
}

void ExecTracker::Untrack(string addr) {
	creation_loc_.erase(addr);
}

void ExecTracker::Print(ostream& out) const {
	for (map<string, string>::const_iterator it = creation_loc_.begin();
		it != creation_loc_.end(); it++) {
		out << "(" << it->first << ", " << it->second << ")" << endl;
	}
}

void ExecTracker::PrintLocs(ostream& out) const {
	map<string, int> locs;
	for (map<string, string>::const_iterator it = creation_loc_.begin();
		it != creation_loc_.end(); it++) {
		locs[it->second]++;
	}
	out << "Locs:";
	for (map<string, int>::iterator loc = locs.begin(); loc != locs.end(); loc++)
		out << " (" << loc->first << ", " << loc->second << ")";
	out << endl;
}

} // namespace despot
