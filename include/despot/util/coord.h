#ifndef COORD_H
#define COORD_H

#include <string>
#include <iostream>

namespace despot {

struct Coord {
	int x, y;

	Coord();
	Coord(int _x, int _y);

	Coord operator*(int v) const;
	friend Coord& operator +=(Coord& left, const Coord& right);
	friend const Coord operator +(const Coord& first, const Coord& second);
	friend bool operator ==(const Coord& first, const Coord& second);
	friend bool operator !=(const Coord& first, const Coord& second);
	friend std::ostream& operator<<(std::ostream& os, const Coord& coord);

	static double EuclideanDistance(Coord c1, Coord c2);
	static int ManhattanDistance(Coord c1, Coord c2);
	static int DirectionalDistance(Coord c1, Coord c2, int direction);
};

/*---------------------------------------------------------------------------*/

struct Compass {
	enum {
		NORTH, EAST, SOUTH, WEST, NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST
	};

	static const Coord DIRECTIONS[];
	static const std::string CompassString[];
	static int Opposite(int dir);
	static bool Opposite(int dir1, int dir2);
};

} // namespace despot

#endif
