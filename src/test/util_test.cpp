#include "test/util_test.h"

void TestSortByValue() {
	map<string, double> map;
	map["p1"] = 2.0;
	map["p2"] = 1.0;
	map["p3"] = 1.0;
	map["p4"] = 1.1;
	map["p5"] = 3.1;

	vector<pair<string, double> > v = SortByValue(map);
	cout << v << endl;
}

void TestSortByKey() {
	map<string, double> map;
	map["p1"] = 2.0;
	map["p3"] = 1.0;
	map["p2"] = 3.0;
	map["p4"] = 1.1;
	map["p5"] = 3.1;

	vector<pair<string, double> > v = SortByKey(map);
	cout << v << endl;
}
