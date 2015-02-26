#pragma once

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

// Data storage format of points.
// Anything that implements .data() and the std tuple interface. (std::get and friends)
typedef std::array<double, 3> RawPoint;

// TODO: (maybe) implement tuple interface (std::get, etc...)
// TODO: (maybe) have more meaningfully named member funcs (x(), y(), z() for Cartesian,
//       radius(), azimuth() etc. for Spherical...).  Perhaps possible through CRTP?
template <class Basis>
class Point
{
public:

	Point (double a, double b, double c, Basis basis)
	: _coords {{a, b, c}}
	, _basis(basis)
	{ }

	Point (RawPoint raw, Basis basis)
	: _coords(raw)
	, _basis(basis)
	{ }

	// member function version of transform
	template <class NewBasis>
	Point<NewBasis> transform (NewBasis newBasis) const {
		// Delegate to the free function form
		return ::transform(*this, newBasis);
	}

	RawPoint & as_raw () { return _coords; }
	const RawPoint & as_raw () const { return _coords; }

	double * data () { return _coords.data(); }
	const double * data () const { return _coords.data(); }

	Basis & basis () { return _basis; }
	Basis basis () const { return _basis; }

	// not sure how I feel about these; specialized names for each basis would be frendlier
	double & first  () { return std::get<0>(_coords); }
	double & second () { return std::get<1>(_coords); }
	double & third  () { return std::get<2>(_coords); }
	double first  () const { return std::get<0>(_coords); }
	double second () const { return std::get<1>(_coords); }
	double third  () const { return std::get<2>(_coords); }

private:
	RawPoint _coords;
	Basis _basis;
};

//--------------------------------------
// A function-style constructor where the type of Basis can be inferred.
// e.g.
// make_point(a, b, c, Cartesian())
// make_point(a, b, c, my_cool_basis)
template <class Basis>
Point<Basis> make_point (double a, double b, double c, Basis basis) {
	return Point<Basis>(a, b, c, basis);
}

template <class Basis>
Point<Basis> tag_point (RawPoint raw, Basis basis) {
	return Point<Basis>(raw, basis);
}

//--------------------------------------
// Definitions of transform()

// This is our "fallback" converter.  When converting between two bases
//  with no specialized conversion function, it will convert to Cartesian
//  first, then to the new type.
template <class FromBasis, class ToBasis>
Point<ToBasis> transform (const Point<FromBasis> & point, ToBasis basis)
{
	return transform(transform(point, Cartesian{}), basis);
}

// The Cartesian => Cartesian trivial conversion:
auto transform (const Point<Cartesian> & point, Cartesian basis) -> Point<decltype(basis)>
{
	return point;
}
