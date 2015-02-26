#pragma once

#include "point.hpp"

//--------------------------------------
// Forward declarations

template <class Basis> class PointCollection;

// Templates for the free-function version of transform.
// Implementations of this function are provided through template specializations.

// This has a default implementation, which uses the transformations implemented for Point.
template <class FromBasis, class ToBasis>
PointCollection<ToBasis> transform (const PointCollection<FromBasis> & point, ToBasis basis);

//--------------------------------------

template <class Basis>
class PointCollection
: private std::vector<RawPoint>
{
private:
	typedef std::vector<RawPoint> container;

public:
	typedef container::size_type       size_type;
	typedef container::difference_type difference_type;

	// RawPoint, RawPoint*, RawPoint&, etc.
	typedef container::value_type      value_type;
	typedef container::pointer         pointer;
	typedef container::reference       reference;
	typedef container::const_pointer   const_pointer;
	typedef container::const_reference const_reference;

	PointCollection (Basis basis)
	: container()
	, _basis(basis)
	{ }

	PointCollection (size_type n, Basis basis)
	: container(n)
	, _basis(basis)
	{ }

	// member function version of transform
	template <class NewBasis>
	PointCollection<NewBasis> transform (NewBasis newBasis) const {
		// Delegate to the free function form
		return ::transform(*this, newBasis);
	}

	// TODO: point() and raw() currently return by value because there is no (easy) way to
	//       allow mutability through point() (this would require something funny like a
	//         ReferencePoint type that holds a RawPoint reference...).
	//       This may change in the future but it's not high priority.

	//       For now, the preferred interface for modifying elements is operator[] and at()
	//       (exposed from the underlying vector implementation)

	// Index methods that return a complete Point.
	Point<Basis> point    (size_type i) const { return tag_point(operator[](i), _basis); }
	Point<Basis> point_at (size_type i) const { return tag_point(at(i), _basis); }

	// Index methods which return RawPoints.
	// For consistency with point(), these return by value.
	inline value_type raw    (size_type i) const { return operator[](i); };
	inline value_type raw_at (size_type i) const { return at(i); };

public:
	// Provide a reasonable subset of vector's interface

	//using container::assign;        // what about basis object?
	//using container::get_allocator; // would rather not get into this yet
	
	using container::at;
	using container::operator[];
	using container::front;
	using container::back;
	//using container::data;     // debating this

	// TODO not sure if I want vector's iterators or if I
	//      might want my own...

	//using container::begin;    // returns iterator
	//using container::cbegin;   // returns iterator
	//using container::end;      // returns iterator
	//using container::cend;     // returns iterator
	//using container::rbegin;   // returns iterator
	//using container::crbegin;  // returns iterator
	//using container::rend;     // returns iterator
	//using container::crend;    // returns iterator

	using container::empty;
	using container::size;
	using container::max_size;
	using container::reserve;
	using container::capacity;
	using container::shrink_to_fit;

	using container::clear;
	//using container::insert;    // uses iterators
	//using container::emplace;   // uses iterators
	//using container::erase;     // uses iterators
	using container::push_back;
	using container::emplace_back;
	using container::pop_back;
	using container::resize;
	//using container::swap;      // what about basis?

	// not sure how I feel about these; specialized names for each basis would be frendlier
	//double & first  (size_type i) { return std::get<0>(_coords[i]); }
	//double & second (size_type i) { return std::get<1>(_coords[i]); }
	//double & third  (size_type i) { return std::get<2>(_coords[i]); }
	//double first  (size_type i) const { return std::get<0>(_coords[i]); }
	//double second (size_type i) const { return std::get<1>(_coords[i]); }
	//double third  (size_type i) const { return std::get<2>(_coords[i]); }

private:
	Basis _basis;
};

//--------------------------------------
// make_point_collection(Cartesian())
// make_point_collection(my_cool_basis)

template <class Basis>
PointCollection<Basis> make_point_collection (Basis basis) {
	return PointCollection<Basis>(basis);
}

//--------------------------------------
// Definitions of transform()

// "Fallback" converter for PointCollection, which converts each of the
//  points using the corresponding conversion for Point objects.
template <class FromBasis, class ToBasis>
PointCollection<ToBasis> transform (const PointCollection<FromBasis> & collection, ToBasis basis)
{
	// FIXME: Could use benchmarking to make this gets optimized well?

	PointCollection<ToBasis> result(collection.size(), basis);
	
	// Transform the raw data for each point via a temp Point object
	for (std::size_t i=0; i<collection.size(); ++i)
		result[i] = transform(collection.point(i), basis).as_raw();

	return result;
}

// TODO Unfortunately, due to the nature of overload matching and resolution (and the fact
//  that PointCollection must already be declared), this function must return by value,
//  which means it must produce a copy.  This kinda sucks?

// The Cartesian => Cartesian trivial conversion:
template <>
auto transform (const PointCollection<Cartesian> & collection, Cartesian basis) -> PointCollection<decltype(basis)>
{
	return collection; // calls copy constructor
}
