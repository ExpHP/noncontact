#pragma once

#ifndef PI
#define PI 3.14159265358979323846
#endif

#include <cassert>

//--------------------------------------
// A basis so important we can't declare anything without it!
class Cartesian { };

//--------------------------------------
// Forward declarations

template <class Basis> class Point;

// Templates for the free-function version of transform.
// Implementations of this function are provided through template specializations.
// Unfortunately, this defers the errors for any missing implementations until the
// linking stage, which are not as helpful as enable_if errors. (FIXME?)

// For conversions between two non-Cartesian types.
// This has a default implementation, which uses Cartesian as an intermediate format.
template <class FromBasis, class ToBasis>
Point<ToBasis> transform (const Point<FromBasis> & point, ToBasis basis);

// For conversions to and from Cartesian.
// These are REQUIRED to be specialized for each Basis.
// They do NOT have a default implementation.
// (note: while the templates themselves aren't strictly necessary, they are here to
//   prevent Cartesian transformations from matching the above template and causing
//   an infinite loop in the event that one of the specializations is missing.)
template <class FromBasis>
Point<Cartesian> transform (const Point<FromBasis> & point, Cartesian basis);
template <class ToBasis>
Point<ToBasis> transform (const Point<Cartesian> & point, ToBasis basis);

// For Cartesian to Cartesian.
// This exists to resolve an ambiguous match (the above two templates tie)
auto transform (const Point<Cartesian> & point, Cartesian basis) -> Point<decltype(basis)>;

//--------------------------------------

// TODO: (maybe) implement tuple interface (std::get, etc...)
// TODO: (maybe) have more meaningfully named member funcs (x(), y(), z() for Cartesian,
//       radius(), azimuth() etc. for Spherical...).  Perhaps possible through CRTP?
template <class Basis>
class Point
{
public:

	Point (double a, double b, double c, Basis basis)
	: _a(a), _b(b), _c(c)
	, _basis(basis)
	{ }

	// member function version of transform
	template <class NewBasis>
	Point<NewBasis> transform (NewBasis newBasis) const {
		// Delegate to the free function form
		return ::transform(*this, newBasis);
	}

	// not sure how I feel about these; specialized names for each basis would be frendlier
	double & first  () { return _a; }
	double & second () { return _b; }
	double & third  () { return _c; }
	double first  () const { return _a; }
	double second () const { return _b; }
	double third  () const { return _c; }

private:
	double _a,_b,_c;
	Basis _basis;
};

//--------------------------------------
// Some Bases
struct Cylindrical { };
struct Spherical   { };

//--------------------------------------
// A function-style constructor where the type of Basis can be inferred.
// e.g.
// make_point(a, b, c, Cartesian())
// make_point(a, b, c, my_cool_basis)
template <class Basis>
Point<Basis> make_point (double a, double b, double c, Basis basis) {
	return Point<Basis>(a, b, c, basis);
}

//--------------------------------------
// Default definition of transform

// This is our "fallback" converter.  When converting between two bases
//  with no specialized conversion function, it will convert to Cartesian
//  first, then to the new type.
template <class FromBasis, class ToBasis>
Point<ToBasis> transform (const Point<FromBasis> & point, ToBasis basis)
{
	return transform(transform(point, Cartesian{}), basis);
}

//--------------------------------------
// Specializations of transform

// Cartesian -> Cartesian (separate overload, not a template specialization)
auto transform (const Point<Cartesian> & point, Cartesian basis) -> Point<decltype(basis)>
{
	return point;
}

// Cylindrical -> Cylindrical
template<> auto transform (const Point<Cylindrical> & point, Cylindrical basis) -> Point<decltype(basis)>
{
	return point;
}

// Spherical -> Spherical
template<> auto transform (const Point<Spherical> & point, Spherical basis) -> Point<decltype(basis)>
{
	return point;
}

// Cartesian -> Cylindrical
template<> auto transform (const Point<Cartesian> & point, Cylindrical basis) -> Point<decltype(basis)>
{
	return {
		sqrt(point.first() * point.first() + point.second() * point.second()),
		atan2(point.second(), point.first()),
		point.third(),
		basis
	};
}

// Cartesian -> Spherical
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

// Cylindrical -> Cartesian
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

// Spherical -> Cartesian
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
