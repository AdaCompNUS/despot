#include <despot/util/coord.h>
#include <cstdlib>
#include <cmath>
#include <cassert>

using namespace std;

namespace despot {

Coord::Coord() :
	x(0),
	y(0) {
}
Coord::Coord(int _x, int _y) :
	x(_x),
	y(_y) {
}

Coord Coord::operator*(int v) const {
	return Coord(this->x * v, this->y * v);
}

Coord& operator+=(Coord& left, const Coord& right) {
	left.x += right.x;
	left.y += right.y;
	return left;
}

const Coord operator+(const Coord& first, const Coord& second) {
	return Coord(first.x + second.x, first.y + second.y);
}

bool operator==(const Coord& first, const Coord& second) {
	return first.x == second.x && first.y == second.y;
}

bool operator!=(const Coord& first, const Coord& second) {
	return first.x != second.x || first.y != second.y;
}

ostream& operator<<(ostream& os, const Coord& coord) {
	os << "(" << coord.x << ", " << coord.y << ")";
	return os;
}

/*---------------------------------------------------------------------------*/
const Coord Compass::DIRECTIONS[] = { Coord(0, 1), Coord(1, 0), Coord(0, -1),
	Coord(-1, 0), Coord(1, 1), Coord(1, -1), Coord(-1, -1), Coord(-1, 1) };

const string Compass::CompassString[] = { "North", "East", "South", "West",
	"NE", "SE", "SW", "NW" };

double Coord::EuclideanDistance(Coord c1, Coord c2) {
	return sqrt((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y));
}

int Coord::ManhattanDistance(Coord c1, Coord c2) {
	return abs(c1.x - c2.x) + abs(c1.y - c2.y);
}

int Coord::DirectionalDistance(Coord lhs, Coord rhs, int direction) {
	switch (direction) {
	case Compass::NORTH:
		return rhs.y - lhs.y;
	case Compass::EAST:
		return rhs.x - lhs.x;
	case Compass::SOUTH:
		return lhs.y - rhs.y;
	case Compass::WEST:
		return lhs.x - rhs.x;
	default:
		assert(false);
		return -1;
	}
}

int Compass::Opposite(int dir) {
	return (dir + 2) % 4;
}

bool Compass::Opposite(int dir1, int dir2) {
	return DIRECTIONS[dir1] + DIRECTIONS[dir2] == Coord(0, 0);
}

} // namespace despot
