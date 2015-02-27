#pragma once

#include "point.hpp"

//--------------------------------------
// Forward declarations

template <class Basis> class PointCollection;

typedef std::vector<RawPoint> RawPointCollection;

// Templates for the free-function version of transform.
// Implementations of this function are provided through template specializations.

template <class InIter, class OutIter, class FromBasis, class ToBasis>
void transform_range (InIter pointsBegin, InIter pointsEnd, OutIter out, const FromBasis &, const ToBasis &);

//--------------------------------------

template <class Basis>
class PointCollection
: private RawPointCollection
{
private:
	typedef RawPointCollection container;

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

	PointCollection (RawPointCollection src, Basis basis)
	: container(src)
	, _basis(basis)
	{ }

	// member function version of transform
	template <class NewBasis>
	PointCollection<NewBasis> transform (NewBasis newBasis) const {
		// Delegate to transform_range
		RawPointCollection newRaw(this->size());
		transform_range(this->begin(), this->end(), newRaw.begin(), basis(), newBasis);

		return PointCollection<NewBasis>(newRaw, newBasis);
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

	Basis basis () const { return _basis; }

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
//  points using the single-point conversion method.
// FIXME: This works but there's no way to override it! (function templates can't be partially specialized)
template <class InIter, class OutIter, class FromBasis, class ToBasis>
void transform_range (InIter pointsBegin, InIter pointsEnd, OutIter out, const FromBasis & fromBasis, const ToBasis & toBasis)
{
	// the standard library function for calling a function on a range is, suitably enough, "transform"...
	// (TODO: we really need our own namespace...)
	std::transform(
		pointsBegin, pointsEnd, out,
		[&] (const RawPoint & p) { return transform(p, fromBasis, toBasis); }
	);
}


/*
// The Cartesian => Cartesian trivial conversion:
// XXX FIXME to my understanding this will overload the template, not specialize it!
template <class InIter, class OutIter>
void transform_range (InIter begin, InIter end, OutIter out, const Cartesian &, const Cartesian &)
{
	std::copy(begin, end, out);
}
*/

