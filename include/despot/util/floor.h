#ifndef FLOOR_H
#define FLOOR_H

#include <despot/util/coord.h>
#include <vector>

namespace despot {

class Floor {
private:
	int num_rows_, num_cols_;
	int** floor_;
  std::vector<Coord> cells_;
	std::vector<std::vector<double> > dist_;

	std::vector<double> ComputeDistances(int source);

public:
	static int INVALID;

	Floor();
	Floor(int num_rows, int num_cols);

	void AddCell(Coord coord);
	Coord GetCell(int i) const;
	inline int GetIndex(const Coord& coord) const {
		return GetIndex(coord.x, coord.y);
	}
	inline int GetIndex(int x, int y) const {
		return
			(x >= 0 && x < num_cols_ && y >= 0 && y < num_rows_) ?
				floor_[y][x] : INVALID;
	}
	bool Inside(Coord coord) const;
	bool Inside(int x, int y) const;

	void ComputeDistances();
	double Distance(int c1, int c2) const;

	std::vector<int> ComputeShortestPath(int start, int end) const;

	int num_rows() const;
	int num_cols() const;
	int NumCells() const;
};

} // namespace despot

#endif
