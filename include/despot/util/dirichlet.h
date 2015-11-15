#ifndef DIRICHLET_H
#define DIRICHLET_H

#include <vector>
#include <despot/util/gamma.h>

class Dirichlet {
private:
  std::vector<double> alpha_;
public:
	Dirichlet(std::vector<double> alpha);

	std::vector<double> alpha();
	std::vector<double> Next();
	static std::vector<double> Next(std::vector<double> alpha);
};

#endif

