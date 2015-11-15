#include <despot/util/floor.h>
#include <queue>

using namespace std;

namespace despot {

int Floor::INVALID = -1;

Floor::Floor() :
	num_rows_(0),
	num_cols_(0) {
}

Floor::Floor(int num_rows, int num_cols) :
	num_rows_(num_rows),
	num_cols_(num_cols) {
	floor_ = new int*[num_rows_];
	for (int r = 0; r < num_rows_; r++) {
		floor_[r] = new int[num_cols_];
		for (int c = 0; c < num_cols_; c++)
			floor_[r][c] = INVALID;
	}
}

void Floor::AddCell(Coord coord) {
	floor_[coord.y][coord.x] = cells_.size();
	cells_.push_back(coord);
}

Coord Floor::GetCell(int i) const {
	return cells_[i];
}

bool Floor::Inside(Coord coord) const {
	return Inside(coord.x, coord.y);
}
bool Floor::Inside(int x, int y) const {
	return (x >= 0 && x < num_cols_ && y >= 0 && y < num_rows_)
		&& floor_[y][x] != INVALID;
}

vector<double> Floor::ComputeDistances(int source) {
	vector<double> dist;
	double** distMatrix = new double*[num_rows_];
	for (int r = 0; r < num_rows_; r++) {
		distMatrix[r] = new double[num_cols_];
		for (int c = 0; c < num_cols_; c++)
			distMatrix[r][c] = NumCells();
	}
	distMatrix[GetCell(source).y][GetCell(source).x] = 0.0;

	queue<Coord> list;
	list.push(GetCell(source));
	while (!list.empty()) {
		Coord cur = list.front();
		list.pop();

		for (int dir = 0; dir < 4; dir++) {
			Coord next = cur + Compass::DIRECTIONS[dir];
			if (GetIndex(next) != INVALID
				&& distMatrix[next.y][next.x] > distMatrix[cur.y][cur.x] + 1) {
				distMatrix[next.y][next.x] = distMatrix[cur.y][cur.x] + 1;
				list.push(next);
			}
		}
	}

	/*
	 for (int r=0; r<num_rows_; r++) {
	 for (int c=0; c<num_cols_; c++)
	 cout << distMatrix[r][c] << " ";
	 cout << endl;
	 }
	 cout << endl;
	 */

	for (int c = 0; c < NumCells(); c++)
		dist.push_back(distMatrix[GetCell(c).y][GetCell(c).x]);

	return dist;
}
void Floor::ComputeDistances() {
	for (int c = 0; c < NumCells(); c++)
		dist_.push_back(ComputeDistances(c));
}

double Floor::Distance(int c1, int c2) const {
	return dist_[c1][c2];
}

vector<int> Floor::ComputeShortestPath(int start, int end) const {
	int cur = start;
	vector<int> moves;
	while (cur != end) {
		int move = 0;
		double cur_dist = Distance(cur, end);

		if (cur_dist == NumCells()) {
			return moves;
		}

		for (int a = 0; a < 4; a++) {
			Coord pos = GetCell(cur) + Compass::DIRECTIONS[a];
			int next = cur;
			if (Inside(pos))
				next = GetIndex(pos);

			double dist = Distance(next, end);

			if (dist < cur_dist) {
				move = a;
				cur = next;
				break;
			}
		}
		moves.push_back(move);
	}
	return moves;
}

int Floor::num_rows() const {
	return num_rows_;
}
int Floor::num_cols() const {
	return num_cols_;
}
int Floor::NumCells() const {
	return cells_.size();
}

} // namespace despot
