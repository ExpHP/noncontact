#pragma once

#include "point.hpp"

#ifndef PI
#define PI 3.14159265358979323846
#endif

// No data members
struct Spherical { };

// Spherical => Cartesian
template<> auto transform (const Point<Spherical> & point, Cartesian basis) -> Point<decltype(basis)>
{
	double cos_theta = cos(point.second());
	double sin_theta = sin(point.second());
	double cos_phi   = cos(point.third());
	double sin_phi   = sin(point.third());
	return {
		point.first() * sin_theta * cos_phi,
		point.first() * sin_theta * sin_phi,
		point.first() * cos_theta,
		basis
	};
}

// Cartesian => Spherical
template<> auto transform (const Point<Cartesian> & point, Spherical basis) -> Point<decltype(basis)>
{
	double rho_sq = point.first() * point.first() + point.second() * point.second();
	double r_sq   = rho_sq + point.third() * point.third();
	return {
		sqrt(r_sq),
		0.5*PI - atan2(point.third(), sqrt(rho_sq)),
		atan2(point.second(), point.first()),
		basis
	};
}

// Spherical => Spherical
template<> auto transform (const Point<Spherical> & point, Spherical basis) -> Point<decltype(basis)>
{
	// Trivial because all Spherical objects are identical.
	return point;
}
