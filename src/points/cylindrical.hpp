#pragma once

#include "point.hpp"

#ifndef PI
#define PI 3.14159265358979323846
#endif

// No data members
struct Cylindrical { };

// Cylindrical => Cartesian
template<> auto transform (const Point<Cylindrical> & point, Cartesian basis) -> Point<decltype(basis)>
{
	double cos_phi = cos(point.second());
	double sin_phi = sin(point.second());
	return {
		point.first() * cos_phi,
		point.first() * sin_phi,
		point.third(),
		basis
	};
}

// Cartesian => Cylindrical
template<> auto transform (const Point<Cartesian> & point, Cylindrical basis) -> Point<decltype(basis)>
{
	return {
		sqrt(point.first() * point.first() + point.second() * point.second()),
		atan2(point.second(), point.first()),
		point.third(),
		basis
	};
}

// Cylindrical => Cylindrical
template<> auto transform (const Point<Cylindrical> & point, Cylindrical basis) -> Point<decltype(basis)>
{
	// Trivial because all Cylindrical objects are identical.
	return point;
}
