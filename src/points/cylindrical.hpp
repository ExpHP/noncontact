#pragma once

#include "point.hpp"

#ifndef PI
#define PI 3.14159265358979323846
#endif

// No data members
struct Cylindrical { };

// Cylindrical => Cartesian
template<> RawPoint transform (const RawPoint & point, const Cylindrical &, const Cartesian &)
{
	double cos_phi = cos(point.second());
	double sin_phi = sin(point.second());
	return {
		point.first() * cos_phi,
		point.first() * sin_phi,
		point.third(),
	};
}

// Cartesian => Cylindrical
template<> RawPoint transform (const RawPoint & point, const Cartesian &, const Cylindrical &)
{
	return {
		sqrt(point.first() * point.first() + point.second() * point.second()),
		atan2(point.second(), point.first()),
		point.third(),
	};
}

// Cylindrical => Cylindrical
template<> RawPoint transform (const RawPoint & point, const Cylindrical &, const Cylindrical &)
{
	// Trivial because all Cylindrical objects are identical.
	return point;
}
