#pragma once

#include <array>
#include <cmath>

#include <Eigen/Dense>

#include "point.hpp"

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
template<> RawPoint transform (const RawPoint & point, const ScaledCartesian & basis, const Cartesian &)
{
	return {
		point.first()  * basis.scale(),
		point.second() * basis.scale(),
		point.third()  * basis.scale(),
	};
}

// Cartesian => ScaledCartesian
template<> RawPoint transform (const RawPoint & point, const Cartesian &, const ScaledCartesian & basis)
{
	return {
		point.first()  / basis.scale(),
		point.second() / basis.scale(),
		point.third()  / basis.scale(),
	};
}

// ScaledCartesian => ScaledCartesian
template<> RawPoint transform (const RawPoint & point, const ScaledCartesian & fromBasis, const ScaledCartesian & toBasis)
{
	double overallScale = fromBasis.scale() / toBasis.scale();
	return {
		point.first()  * overallScale,
		point.second() * overallScale,
		point.third()  * overallScale,
	};
}
