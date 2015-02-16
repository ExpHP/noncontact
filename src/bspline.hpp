#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <Eigen/Dense>

#include "util/index-iterator.hpp"

// FIXME: `using` in header = badness
using namespace Eigen;

template <typename Coord, typename Data>
class BSpline {
	public:

		// INVARIANT:  num_knots() == num_points() + degree() + 1
		// INVARIANT:  knots form a non-decreasing sequence

		// Default constructor
		template <typename KnotIterator>
		BSpline (KnotIterator knotFirst, KnotIterator knotLast, unsigned degree)
		: _degree(degree)
		, _knots(knotFirst, knotLast)
		, _points((knotLast - knotFirst) - degree - 1, Data{})
		{
			std::sort(_knots.begin(), _knots.end());
		}

		// FIXME
		template <typename DataInputIt, typename CoordInputIt, typename KnotInputIt>
		static BSpline from_data (
			CoordInputIt coordBegin, CoordInputIt coordEnd, // Independent var
			DataInputIt  dataBegin,  DataInputIt  dataEnd,  // Dependent var
			KnotInputIt  knotsBegin, KnotInputIt  knotsEnd, // Desired knot locations for spline
			unsigned degree // Order of the polynomials used to interpolate values between knots
		) {

			// Check preconditions
			if (coordEnd - coordBegin != dataEnd - dataBegin)
				throw std::runtime_error("Data and coords length mismatch");

			auto dataSize = dataEnd - dataBegin;

			// Create a BSpline
			BSpline spline(knotsBegin, knotsEnd, degree);

			// Product vector, which contains the data
			Matrix<Data,Dynamic,1> bvec {dataSize, 1};

			std::copy(dataBegin, dataEnd, index_begin<Data>(bvec));

			// Coefficient matrix - spline coeffs computed at each coord
			Matrix<Coord,Dynamic,Dynamic> amat {dataSize, spline.num_points()};
			for (long i=0l; i<dataSize; i++) {

				Coord x  = *(coordBegin++);
				auto out = index_begin<Coord>(amat.row(i)); // output iterator

				spline.compute_coeffs(x, out);
			}

			// TODO Once I have means of benchmarking:
			// When size > degree, the matrix for this system is banded with (degree+1) bands.
			// While Eigen doesn't support the banded matrix structure explicitly, it does have
			//  a Sparse-matrix QR decomposition module (SPQR) which may be worth a try.

			// TODO Question of stability:
			// De Boor's algorithm for evaluating BSplines is frequently cited as "stable",
			//  implying that other methods suffer from numerical instability.
			// De Boor's algorithm could *not* be applied here, since if we want to represent the
			//  problem as a linear least squares problem, the coefficients must be independent of
			//  the control points.
			// What issues do BSplines have with stability, and do they affect us here?

			// Solve the least squares system
			auto solver = amat.colPivHouseholderQr();
			Matrix<Data, Dynamic, 1> soln = solver.solve(bvec);

			assert(soln.rows() == spline.num_points());

			// Populate spline._points
			std::copy(index_begin<Data>(soln), index_end<Data>(soln), spline._points.begin());
		}
 
		Data interpolate (Coord x) const {
			// Use the De Boor algorithm
			long region = region_index(x);

			// Gather the points involved in interpolating this region.
			// Some points may have negative indices, which we leave as zero.
			Data zero = Data(); // any reliable way to get an additive identity?
			std::vector<Data> values(_degree + 1, zero);

			// points involved are from `first` to `region` (inclusive)
			long first = region - (long)_degree;

			// Copy elements in this range with positive indices
			long srcOffset = std::max(0l, first);
			long destOffset = srcOffset - first;
			long toCopy = (long)_degree + 1 - destOffset;

			assert(srcOffset  + toCopy == region + 1);
			assert(srcOffset  + toCopy <= _points.size());
			assert(destOffset + toCopy == values.size());
	
			std::copy(
				&_points[srcOffset], &_points[srcOffset+toCopy],
				&values[destOffset]
			);

			for (long rem = _degree; rem > 0; --rem) {

				// Each iteration, update the last `rem` points.
				// Work from right to left due to interdependencies
				for (long cur = region; cur > region-rem; --cur) {
					Coord numer =                 x - _knots[cur];
					Coord denom = _knots[cur + rem] - _knots[cur];

					Coord ratio = numer / denom;

					// Update in place
					long valueI = cur - first;
					values[valueI] *= ratio;
					values[valueI] += (1.0 - ratio) * values[valueI-1];
				}
			}

			// Last element of values holds the result
			return values.back();
		}

		long degree ()     const { return _degree; }
		long num_points () const { return _points.size(); }
		long num_knots ()  const { return _knots.size(); }

		Data& point (std::size_t i)       { return _points[i]; }
		Data  point (std::size_t i) const { return _points[i]; }

		Coord knot (std::size_t i) const { return _knots[i]; }
		// a knot setter would allow the user to violate the "sorted" invariant

	private:

		long region_index (Coord x) const
		{	
			// Locate last knot not greater than x
			auto it = std::upper_bound(_knots.begin(), _knots.end(), x) - 1;

			// Convert to index
			return it - _knots.begin();
		}

		// FIXME
		template <typename OutputIterator>
		void compute_coeffs (Coord x, OutputIterator dest) const
		{
			// double check invariant
			assert(num_points() + degree() + 1 == num_knots());

			// to simplify computations, we'll have a coefficient for each knot, even though
			//  our final output will only contain the first num_points() entries
			std::vector<Coord> coeffs(num_knots(), 0.0);
			auto resultBegin = coeffs.begin();
			auto resultEnd   = coeffs.begin() + num_points();

			// For degree 0, only one coefficient (x's region) is nonzero.
			long region = region_index(x);
			coeffs[region] = 1.0;

			// For larger degrees, the coefficients are defined recursively.
			// We compute them in a manner that works a bit like De Boor's algorithm, in reverse.
			for (long d=1; d <= _degree; ++d) {

				// For degree d there are exactly d+1 nonzero coefficients.
				// However, some might have negative indices (and we don't need them!).
				long start = std::max(0l, region-d);
				for (long cur=start; cur <= region; ++cur) {
					Coord numer1 =             x - _knots[cur];
					Coord denom1 = _knots[cur+d] - _knots[cur];

					Coord numer2 = _knots[cur+d+1] - x;
					Coord denom2 = _knots[cur+d+1] - _knots[cur+1];

					// Update in place
					coeffs[cur] *= (numer1 / denom1);
					coeffs[cur] += (numer2 / denom2) * coeffs[cur+1];
				}

				// TODO Slightly more efficient version to bench and compare results
				// (this version eliminates half of the divisions above, as (numer2/denom2)
				//  and (numer1/denom1) from successive iterations add up to 1)
				/*
				long start = max(0l, region-d+1);
				for (long cur=start; cur <= region; ++cur) {
					Coord numer =             x - _knots[cur];
					Coord denom = _knots[cur+d] - _knots[cur];

					Coord ratio = numer / denom;

					coeffs[cur-1] += (1.0-ratio) * coeffs[cur];
					coeffs[cur]   *= ratio;
				}
				*/
			}

			std::copy(resultBegin, resultEnd, dest);
		}

		unsigned long      _degree;
		std::vector<Coord> _knots;
		std::vector<Data>  _points;
};
