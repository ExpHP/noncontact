#pragma once

#include <array>
#include <cmath>

#include <Eigen/Dense>

#include "../points.hpp"

// Cartesian coordinates, divided by a uniform scale factor.
// (To be crystal clear: we multiply when converting TO cartesian,
//   and divide when converting FROM cartesian)
struct ScaledCartesian
{
public:
	ScaledCartesian (double scale)
	: _scale(scale)
	{ }

	double scale () const { return _scale; }

private:
	double _scale;
};

// ScaledCartesian => Cartesian
template<> auto transform (const Point<ScaledCartesian> & point, Cartesian basis) -> Point<decltype(basis)>
{
	return {
		point.first()  * point.basis().scale(),
		point.second() * point.basis().scale(),
		point.third()  * point.basis().scale(),
		basis
	};
}

// Cartesian => ScaledCartesian
template<> auto transform (const Point<Cartesian> & point, ScaledCartesian basis) -> Point<decltype(basis)>
{
	return {
		point.first()  / basis.scale(),
		point.second() / basis.scale(),
		point.third()  / basis.scale(),
		basis
	};
}

// ScaledCartesian => ScaledCartesian
template<> auto transform (const Point<ScaledCartesian> & point, ScaledCartesian basis) -> Point<decltype(basis)>
{
	double overallScale = point.basis().scale() / basis.scale();
	return {
		point.first()  * overallScale,
		point.second() * overallScale,
		point.third()  * overallScale,
		basis
	};
}
