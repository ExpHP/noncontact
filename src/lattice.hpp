#pragma once

#include "external-deps/catch.hpp"

// BEWARE:  `ERE THE MIGHTY COPY-PASTA SLEEPS

// TODO: Having duplicated code like this makes it easy to introduce bugs.
// How could this be simplified while still allowing axes for methods
//  to be specified at compile-time?

template <typename T>
class Lattice2 {
	public:

		// value-related typedefs
		typedef T              value_type;
		typedef T *            pointer;
		typedef T &            reference;
		typedef const T *      const_pointer;
		typedef const T &      const_reference;

		// index-related typedefs
		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		// coordinate-related typedefs
		typedef T              coord_type;

		// Default constructor
		Lattice2 ()
		: _data()
		, _dims(std::make_tuple(0, 0))
		, _lbs(std::make_tuple(0, 0))
		, _ubs(std::make_tuple(1, 1))
		{ }

		// Sized constructor
		Lattice2 (size_type d1, size_type d2)
		: _data(d1*d2)
		, _dims(std::make_tuple(d1, d2))
		, _lbs(std::make_tuple(0, 0))
		, _ubs(std::make_tuple(1, 1))
		{ }

		// TODO: Does this have default move assignment/constructors?

		// Named parameter idiom for handling the less significant details
		Lattice2 & set_lower_coords (T x, T y) {
			_lbs = std::make_tuple(x,y);
			return *this;
		}

		Lattice2 & set_upper_coords (T x, T y) {
			_ubs = std::make_tuple(x,y);
			return *this;
		}

		// Element access
		inline reference operator() (size_type i, size_type j) {
			return _data[flat_index(i,j)];
		}

		inline const_reference operator() (size_type i, size_type j) const {
			return _data[flat_index(i,j)];
		}

		// Total and axial dimensions
		inline size_type size () { return size_1() * size_2(); }
		inline size_type size_1 () { return std::get<0>(_dims); }
		inline size_type size_2 () { return std::get<1>(_dims); }

		inline coord_type lower_coord_1 () { return std::get<0>(_lbs); }
		inline coord_type lower_coord_2 () { return std::get<1>(_lbs); }

		inline coord_type upper_coord_1 () { return std::get<0>(_ubs); }
		inline coord_type upper_coord_2 () { return std::get<1>(_ubs); }

		inline coord_type length_coord_1 () { return upper_coord_1() - lower_coord_1(); }
		inline coord_type length_coord_2 () { return upper_coord_2() - lower_coord_2(); }

		inline coord_type coord_step_1 () { return length_coord_1() / (size_1()-1); }
		inline coord_type coord_step_2 () { return length_coord_2() / (size_2()-1); }

		inline coord_type coord_at_1 (size_t i) { return lower_coord_1() + i*coord_step_1(); }
		inline coord_type coord_at_2 (size_t i) { return lower_coord_2() + i*coord_step_2(); }

		// if this is to be made public, so should strides and flat_index
//		inline const_pointer data () { return _data.data(); }

		bool operator==(const Lattice2<T> & other) const {
			// deep comparison of all members
			return
				(this->_dims == other._dims)
				&& (this->_lbs == other._lbs)
				&& (this->_ubs == other._ubs)
				&& (this->_data == other._data)
			;
		};

		bool operator!=(const Lattice2<T> & other) const {
			return !(*this == other);
		};

	private:
		std::vector<T> _data;
		std::tuple<size_type, size_type> _dims;  // dimensions
		std::tuple<coord_type, coord_type> _lbs; // lower coordinate bounds
		std::tuple<coord_type, coord_type> _ubs; // upper coordinate bounds

		inline difference_type stride_1 () { return size_2(); }
		inline difference_type stride_2 () { return 1; }

		inline size_type flat_index (size_type i, size_type j) {
			return stride_1()*i + stride_2()*j;
		}

};

// // really don't think static sizes are necessary
//template <typename T, std::size_t D1, std::size_t D2, std::size_t D3>
template <typename T>
class Lattice3 {
	public:

		// value-related typedefs
		typedef T              value_type;
		typedef T *            pointer;
		typedef T &            reference;
		typedef const T *      const_pointer;
		typedef const T &      const_reference;

		// index-related typedefs
		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		// coordinate-related typedefs
		typedef T              coord_type;

		// Default constructor
		Lattice3 ()
		: _data()
		, _dims(std::make_tuple(0, 0, 0))
		, _lbs(std::make_tuple(0, 0, 0))
		, _ubs(std::make_tuple(1, 1, 1))
		{ }

		// Sized constructor
		Lattice3 (size_type d1, size_type d2, size_type d3)
		: _data(d1*d2*d3)
		, _dims(std::make_tuple(d1, d2, d3))
		, _lbs(std::make_tuple(0, 0, 0))
		, _ubs(std::make_tuple(1, 1, 1))
		{ }

		// TODO: Does this have default move assignment/constructors?

		// Initialization methods
		Lattice3 & set_lower_coords (T x, T y, T z) {
			_lbs = std::make_tuple(x,y,z);
			return *this;
		}

		Lattice3 & set_upper_coords (T x, T y, T z) {
			_ubs = std::make_tuple(x,y,z);
			return *this;
		}

		// Element access
		inline reference operator() (size_type i, size_type j, size_type k) {
			return _data[flat_index(i,j,k)];
		}

		inline const_reference operator() (size_type i, size_type j, size_type k) const {
			return _data[flat_index(i,j,k)];
		}

		// Total and axial dimensions
		inline size_type size () { return size_1() * size_2() * size_3(); }
		inline size_type size_1 () { return std::get<0>(_dims); }
		inline size_type size_2 () { return std::get<1>(_dims); }
		inline size_type size_3 () { return std::get<2>(_dims); }

		inline coord_type lower_coord_1 () { return std::get<0>(_lbs); }
		inline coord_type lower_coord_2 () { return std::get<1>(_lbs); }
		inline coord_type lower_coord_3 () { return std::get<2>(_lbs); }

		inline coord_type upper_coord_1 () { return std::get<0>(_ubs); }
		inline coord_type upper_coord_2 () { return std::get<1>(_ubs); }
		inline coord_type upper_coord_3 () { return std::get<2>(_ubs); }

		inline coord_type length_coord_1 () { return upper_coord_1() - lower_coord_1(); }
		inline coord_type length_coord_2 () { return upper_coord_2() - lower_coord_2(); }
		inline coord_type length_coord_3 () { return upper_coord_3() - lower_coord_3(); }

		inline coord_type coord_step_1 () { return length_coord_1() / (size_1()-1); }
		inline coord_type coord_step_2 () { return length_coord_2() / (size_2()-1); }
		inline coord_type coord_step_3 () { return length_coord_3() / (size_3()-1); }

		inline coord_type coord_at_1 (size_t i) { return lower_coord_1() + i*coord_step_1(); }
		inline coord_type coord_at_2 (size_t i) { return lower_coord_2() + i*coord_step_2(); }
		inline coord_type coord_at_3 (size_t i) { return lower_coord_3() + i*coord_step_3(); }

		// if this is to be made public, so should strides and flat_index
//		inline const_pointer data () { return _data.data(); }

		bool operator==(const Lattice3<T> & other) const {
			// deep comparison of all members
			return
				(this->_dims == other._dims)
				&& (this->_lbs == other._lbs)
				&& (this->_ubs == other._ubs)
				&& (this->_data == other._data)
			;
		};

		bool operator!=(const Lattice3<T> & other) const {
			return !(*this == other);
		};

	private:
		std::vector<T> _data;
		std::tuple<size_type, size_type, size_type> _dims;   // dimensions
		std::tuple<coord_type, coord_type, coord_type> _lbs; // lower coordinate bounds
		std::tuple<coord_type, coord_type, coord_type> _ubs; // upper coordinate bounds

		inline difference_type stride_1 () { return size_3() * size_2(); }
		inline difference_type stride_2 () { return size_3(); }
		inline difference_type stride_3 () { return 1; }

		inline size_type flat_index (size_type i, size_type j, size_type k) {
			return stride_1()*i + stride_2()*j + stride_3()*k;
		}

};


TEST_CASE("lattices") {
	CHECK(false); // TODO
}
