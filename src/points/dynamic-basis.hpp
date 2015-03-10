#pragma once

#include "point.hpp"

// FIXME some names here are confusing (the meaning of "into" vs "from" varies)
//        or misleading ("_range_" for methods operating on collections)

struct AnyBasis {
	private:
		// Forward declaration
		class Placeholder;

		// Holds the basis using type erasure
		std::shared_ptr<Placeholder> _holder;

	public:
		// AnyBasis(basis) -- constructs an AnyBasis from a basis of known type
		template <class Basis>
		explicit AnyBasis (const Basis & basis)
		: _holder(std::make_shared<Holder<Basis>>(basis))
		{ }

		// Use implicit copy constructor and copy assignment

		// Methods for dynamically transforming points;
		// Delegated to the Holder class.
		template <class Basis>
		RawPoint transform_point_from_self (RawPoint point, const Basis & basis) const {
			return (*_holder).bind_transform_into(point, basis);
		}

		template <class Basis>
		RawPointCollection transform_collection_from_self (RawPointCollection points, const Basis & basis) const {
			return (*_holder).bind_transform_range_into(points, basis);
		}

		// From a known basis into this basis:  Delegate to the dynamic dispatch
		// FIXME this could be avoided by overloading bind_x_from similarly to bind_x_into
		//       but I'd rather wait until these methods are automatically generated
		template <class Basis>
		RawPoint transform_point_into_self (RawPoint point, const Basis & basis) const {
			return (*_holder).bind_transform_from(point, AnyBasis(basis));
		}
		template <class Basis>
		RawPointCollection transform_collection_into_self (RawPointCollection points, const Basis & basis) const {
			return (*_holder).bind_transform_range_from(points, AnyBasis(basis));
		}

	private:

		// Interface class for type erasure.
		class Placeholder {
			public:
				virtual ~Placeholder() {}

				// "accept" methods for the visitor pattern, used to simulate double
				//  dispatch when transforming from AnyBasis to another AnyBasis.
				virtual RawPoint           accept_transform_from       (RawPoint, const Placeholder &) const = 0;
				virtual RawPointCollection accept_transform_range_from (RawPointCollection, const Placeholder &) const = 0;

				// Methods for converting from an AnyBasis.
				// There is an overload for each basis, since templates cannot be virtual.
				virtual RawPoint bind_transform_into (RawPoint, const Cartesian &) const = 0;
				virtual RawPoint bind_transform_into (RawPoint, const Spherical &) const = 0;
				virtual RawPoint bind_transform_into (RawPoint, const Cylindrical &) const = 0;
				virtual RawPoint bind_transform_into (RawPoint, const ScaledCartesian &) const = 0;
				virtual RawPoint bind_transform_into (RawPoint, const VectorBasis &) const = 0;
				virtual RawPoint bind_transform_into (RawPoint, const AnyBasis &) const = 0;

				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const Cartesian &) const = 0;
				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const Spherical &) const = 0;
				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const Cylindrical &) const = 0;
				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const ScaledCartesian &) const = 0;
				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const VectorBasis &) const = 0;
				virtual RawPointCollection bind_transform_range_into (RawPointCollection, const AnyBasis &) const = 0;

				// Methods for converting into an AnyBasis
				virtual RawPoint           bind_transform_from       (RawPoint point, const AnyBasis & fromBasis) const = 0;
				virtual RawPointCollection bind_transform_range_from (RawPointCollection points, const AnyBasis & fromBasis) const = 0;
		};

		// Implementation class for type erasure.
		template <class Basis>
		class Holder : public Placeholder
		{
			public:
				explicit Holder(Basis obj)
				: _basis(obj)
				{ }

				virtual ~Holder() = default;

				// "accept" methods for the visitor pattern (used when both bases are dynamic)
				RawPoint accept_transform_from (RawPoint point, const Placeholder & visitor) const {
					return visitor.bind_transform_into(point, _basis);
				}
				RawPointCollection accept_transform_range_from (RawPointCollection points, const Placeholder & visitor) const {
					return visitor.bind_transform_range_into(points, _basis);
				}

				// Methods for converting from an AnyBasis.
				// Most of these are trivial.
				RawPoint bind_transform_into (RawPoint point, const Cartesian & toBasis) const {
					return bind_transform_into_known(point, toBasis);
				}
				RawPoint bind_transform_into (RawPoint point, const Cylindrical & toBasis) const {
					return bind_transform_into_known(point, toBasis);
				}
				RawPoint bind_transform_into (RawPoint point, const Spherical & toBasis) const {
					return bind_transform_into_known(point, toBasis);
				}
				RawPoint bind_transform_into (RawPoint point, const ScaledCartesian & toBasis) const {
					return bind_transform_into_known(point, toBasis);
				}
				RawPoint bind_transform_into (RawPoint point, const VectorBasis & toBasis) const {
					return bind_transform_into_known(point, toBasis);
				}

				RawPointCollection bind_transform_range_into (RawPointCollection points, const Cartesian & toBasis) const {
					return bind_transform_range_into_known(points, toBasis);
				}
				RawPointCollection bind_transform_range_into (RawPointCollection points, const Cylindrical & toBasis) const {
					return bind_transform_range_into_known(points, toBasis);
				}
				RawPointCollection bind_transform_range_into (RawPointCollection points, const Spherical & toBasis) const {
					return bind_transform_range_into_known(points, toBasis);
				}
				RawPointCollection bind_transform_range_into (RawPointCollection points, const ScaledCartesian & toBasis) const {
					return bind_transform_range_into_known(points, toBasis);
				}
				RawPointCollection bind_transform_range_into (RawPointCollection points, const VectorBasis & toBasis) const {
					return bind_transform_range_into_known(points, toBasis);
				}

				// For converting between two AnyBasis objects, use the visitor pattern
				RawPoint bind_transform_into (RawPoint point, const AnyBasis & toBasis) const {
					return (*toBasis._holder).accept_transform_from(point, *this);
				}

				RawPointCollection bind_transform_range_into (RawPointCollection points, const AnyBasis & toBasis) const {
					return (*toBasis._holder).accept_transform_range_from(points, *this);
				}

				// Methods for converting into an AnyBasis
				RawPoint bind_transform_from (RawPoint point, const AnyBasis & fromBasis) const {
					return (*fromBasis._holder).bind_transform_into(point, _basis);
				}
				RawPointCollection bind_transform_range_from (RawPointCollection points, const AnyBasis & fromBasis) const {
					return (*fromBasis._holder).bind_transform_range_into(points, _basis);
				}

			private:
				Basis _basis;

				// The methods which finally lie at the end of all this madness:
				//  Templates which perform transformations into a statically typed basis.

				template <
					class ToBasis,
					typename = typename std::enable_if<!std::is_same<ToBasis,AnyBasis>::value>::type // disable for ToBasis == AnyBasis
				>
				RawPoint bind_transform_into_known (RawPoint point, const ToBasis & toBasis) const {
					return ::transform(point, _basis, toBasis);
				}

				template <
					class ToBasis,
					typename = typename std::enable_if<!std::is_same<ToBasis,AnyBasis>::value>::type // disable for ToBasis == AnyBasis
				>
				RawPointCollection bind_transform_range_into_known (RawPointCollection points, const ToBasis & toBasis) const {
					RawPointCollection result;
					::transform_range(points.begin(), points.end(), std::back_inserter(result), _basis, toBasis);
					return result;
				}
		};

};



//--------------------
// Yet another layer of implementation.
// These are the /intended/ implementations of transform() and transform_range(), but
//  they require unique names so that specific ones can be called in tiebreakers

// into AnyBasis
template <class FromBasis>
RawPoint transform_into_dynamic (const RawPoint & point, const FromBasis & other, const AnyBasis & basis)
{
	return basis.transform_point_into_self(point, other);
}

// from AnyBasis
template <class ToBasis>
RawPoint transform_from_dynamic (const RawPoint & point, const AnyBasis & basis, const ToBasis & other)
{
	return basis.transform_point_from_self(point, other);
}

// into AnyBasis (range)
template <class InIter, class OutIter, class FromBasis>
void transform_range_into_dynamic (InIter pointsBegin, InIter pointsEnd, OutIter out, const FromBasis & other, const AnyBasis & basis)
{
	// TODO:  Is there no other way?  Is there REALLY no other way?
	//        (well, okay, there is boost::any_iterator...)
	RawPointCollection original(pointsBegin, pointsEnd); // copy entire range into a collection :/
	RawPointCollection transformed = basis.transform_collection_into_self(original, other);

	assert(transformed.end() - transformed.begin() == pointsEnd - pointsBegin);
	std::copy(transformed.begin(), transformed.end(), out);
}

// from AnyBasis (range)
template <class InIter, class OutIter, class ToBasis>
void transform_range_from_dynamic (InIter pointsBegin, InIter pointsEnd, OutIter out, const AnyBasis & basis, const ToBasis & other)
{
	RawPointCollection original(pointsBegin, pointsEnd); // copy entire range into a collection :/
	RawPointCollection transformed = basis.transform_collection_from_self(original, other);

	assert(transformed.end() - transformed.begin() == pointsEnd - pointsBegin);
	std::copy(transformed.begin(), transformed.end(), out);
}

//---------------------------
// These implement transform() for the majority of cases that involve AnyBasis

// into AnyBasis
template <class FromBasis>
RawPoint transform (const RawPoint & point, const FromBasis & from, const AnyBasis & to)
{
	return transform_into_dynamic(point, from, to);
}

// from AnyBasis
template <class ToBasis>
RawPoint transform (const RawPoint & point, const AnyBasis & from, const ToBasis & to)
{
	return transform_from_dynamic(point, from, to);
}

// into AnyBasis (range)
template <class InIter, class OutIter, class FromBasis>
void transform_range (InIter begin, InIter end, OutIter out, const FromBasis & from, const AnyBasis & to)
{
	transform_range_into_dynamic(begin, end, out, from, to);
}

// from AnyBasis (range)
template <class InIter, class OutIter, class ToBasis>
void transform_range (InIter begin, InIter end, OutIter out, const AnyBasis & from, const ToBasis & to)
{
	transform_range_from_dynamic(begin, end, out, from, to);
}

//---------------------------
// Tiebreakers --- these implement transform() for arguments that match more than
//                 one of the templated overloads (making them ambiguous)

// Cartesian conversions --- These must use the dynamic versions
RawPoint transform (const RawPoint & point, const Cartesian & from, const AnyBasis & to)
{
	return transform_into_dynamic(point, from, to);
}

RawPoint transform (const RawPoint & point, const AnyBasis & from, const Cartesian & to)
{
	return transform_from_dynamic(point, from, to);
}

//AnyBasis <-> AnyBasis : Just pick one (from_dynamic or into_dynamic)
RawPoint transform (const RawPoint & point, const AnyBasis & other, const AnyBasis & basis)
{
	return transform_from_dynamic(point, other, basis);
}

template <class InIter, class OutIter>
void transform_range (InIter begin, InIter end, OutIter out, const AnyBasis & from, const AnyBasis & to)
{
	transform_range_from_dynamic(begin, end, out, from, to);
}

//---------------------------
//
// ...
//
// ...holy crap.
