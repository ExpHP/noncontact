#pragma once

#include <algorithm>
#include <numeric>

#include "util/all-convertible-to.hpp"

// Adds SFINAE template members to check that the variadic template argument `A`
//  contains `N` elements, all convertible to `T`.
// Used to implement functions like .set_lower_coords(x, y, z)
#define RequireVariadicArray(A,T,N) \
	typename = typename std::enable_if<sizeof...(A)==(N)>::type, \
	typename = typename std::enable_if<all_convertible_to<T, A...>::value>::type

// Adds SFINAE template members to disable methods based on axis N, requiring that `Min <= N <= Max`.
#define RequireAxis(N,Min,Max) \
	axis_type RequestedAxis = (N), /* Dummy parameter */ \
	typename = typename std::enable_if< (RequestedAxis >= (Min)) >::type, \
	typename = typename std::enable_if< (RequestedAxis <= (Max)) >::type

// Generates methods specific to an axis (size_0(), coord_step_0(), ...), which have the
//  distinct advantage of *not existing* if you make a logical error and use an invalid axis.
// (it also eliminates any question about the order of arguments to coord_n())
#define GenerateAxisMethods(N) \
	template <RequireAxis(N, 0, Dim-1)> inline size_type       size_##N ()             const { return size_n(N); } \
	template <RequireAxis(N, 0, Dim-1)> inline coord_type      lower_coord_##N ()      const { return lower_coord_n(N); } \
	template <RequireAxis(N, 0, Dim-1)> inline coord_type      upper_coord_##N ()      const { return upper_coord_n(N); } \
	template <RequireAxis(N, 0, Dim-1)> inline coord_type      coord_length_##N ()     const { return coord_length_n(N); } \
	template <RequireAxis(N, 0, Dim-1)> inline coord_type      coord_step_##N ()       const { return coord_step_n(N); } \
	template <RequireAxis(N, 0, Dim-1)> inline coord_type      coord_##N (size_type i) const { return coord_n(N, i); } \
	template <RequireAxis(N, 0, Dim-1)> inline difference_type stride_##N ()           const { return stride_n(N); }

template <typename T, int Dim>
class Lattice {
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

		// type of dimension number
		typedef decltype(Dim)  axis_type;

		// coordinate-related typedefs
		typedef T              coord_type;

		// std::array constructor
		Lattice (std::array<size_type, Dim> dims)
		: _dims(dims)
		, _strides(compute_strides(_dims))
		, _lbs(default_lower_bounds())
		, _ubs(default_upper_bounds())
		, _data(compute_full_size(_dims))
		{ }

		// variadic constructor
		template <class... Args, RequireVariadicArray(Args, size_type, Dim)>
		Lattice (Args... dims)
		: _dims(make_array<size_type>(dims...))
		, _strides(compute_strides(_dims))
		, _lbs(default_lower_bounds())
		, _ubs(default_upper_bounds())
		, _data(compute_full_size(_dims))
		{ }

		// Named parameter idiom for handling the less significant details
		template <class... Args, RequireVariadicArray(Args, coord_type, Dim)>
		Lattice & set_lower_coords (Args... coords) {
			_lbs = {{coords...}};
			return *this;
		}

		template <class... Args, RequireVariadicArray(Args, coord_type, Dim)>
		Lattice & set_upper_coords (Args... coords) {
			_ubs = {{coords...}};
			return *this;
		}

		// Element access
		template <class... Args, RequireVariadicArray(Args, size_type, Dim)>
		reference operator() (Args... indices) {
			return _data[flat_index(make_array<size_type>(indices...))];
		}

		template <class... Args, RequireVariadicArray(Args, size_type, Dim)>
		const_reference operator() (Args... indices) const {
			return _data[flat_index(make_array<size_type>(indices...))];
		}

		// Total size
		inline size_type size () const { return _data.size(); }

		static constexpr axis_type num_dims() { return Dim; }

		// Per-axis features
		// FIXME:  naming inconsistencies (xxx_coord vs coord_xxx,  and set_xxx should resemble xxx)
		inline size_type       size_n (axis_type n)               const { return _dims[n]; }
		inline coord_type      lower_coord_n (axis_type n)        const { return _lbs[n]; }
		inline coord_type      upper_coord_n (axis_type n)        const { return _ubs[n]; }
		inline coord_type      coord_length_n (axis_type n)       const { return upper_coord_n(n) - lower_coord_n(n); }
		inline coord_type      coord_step_n (axis_type n)         const { return coord_length_n(n) / (size_n(n)-1); }
		inline coord_type      coord_n (axis_type n, size_type i) const { return lower_coord_n(n) + i*coord_step_n(n); }
		inline difference_type stride_n (axis_type n)             const { return _strides[n]; }

		// Generate the numbered variants (_0, _1, _2, ...) of the above methods
		GenerateAxisMethods(0);
		GenerateAxisMethods(1);
		GenerateAxisMethods(2);
		GenerateAxisMethods(3);
		GenerateAxisMethods(4);
		GenerateAxisMethods(5);
		GenerateAxisMethods(6);
		GenerateAxisMethods(7);
		GenerateAxisMethods(8);

		// Waiting on this until I'm convinced that it's a reasonable part of the public API
		// (would make more sense if I had axial iterators)
//		inline const_pointer data () { return _data.data(); }

		/*
		bool operator==(const Lattice<T, Dim> & other) const {
			// deep comparison of all members
			return
				(this->_dims == other._dims)
				&& (this->_lbs == other._lbs)
				&& (this->_ubs == other._ubs)
				&& (this->_data == other._data)
			;
		};

		bool operator!=(const Lattice<T, Dim> & other) const {
			return !(*this == other);
		};
		*/

	private:
		std::array<size_type, Dim>       _dims;    // number of points in each dimension
		std::array<difference_type, Dim> _strides; // index difference in each dimension

		std::array<coord_type, Dim> _lbs; // first coordinate in each dimension
		std::array<coord_type, Dim> _ubs; // last coordinate in each dimension

		std::vector<T> _data; // The meat

		static size_type compute_full_size(const std::array<size_type, Dim> & dims) {
			// Product
			size_type acc = 1;
			for (axis_type i=0; i<Dim; ++i)
				acc *= dims[i];
			return acc;
		}

		static std::array<difference_type, Dim> compute_strides(const std::array<size_type, Dim> & dims) {
			std::array<difference_type, Dim> res;

			res[Dim-1] = 1; // Most minor dimension
			for (axis_type i=Dim-2; i>=0; --i) // Other axes
				res[i] = res[i+1] * dims[i+1];
			return res;
		}

		static std::array<coord_type, Dim> default_lower_bounds() {
			std::array<coord_type, Dim> pt;  pt.fill(0.0);  return pt;
		}

		static std::array<coord_type, Dim> default_upper_bounds() {
			std::array<coord_type, Dim> pt;  pt.fill(1.0);  return pt;
		}

		// Used to convert a variadic argument list into std::array<OutType>,
		//  casting each element in the process
		template <class OutType, class... Args, RequireVariadicArray(Args, OutType, Dim)>
		static std::array<OutType, Dim> make_array(Args... args) {
			return std::array<OutType, Dim> {{static_cast<OutType>(args)...}};
		}

		inline size_type flat_index (const std::array<size_type, Dim> & indices) {
			return std::inner_product(
				indices.begin(),  indices.end(),
				_strides.begin(),
				0
			);
		}
};

// Shorthands
template <class T> using Lattice1 = Lattice<T,1>;
template <class T> using Lattice2 = Lattice<T,2>;
template <class T> using Lattice3 = Lattice<T,3>;
template <class T> using Lattice4 = Lattice<T,4>;
template <class T> using Lattice5 = Lattice<T,5>;
template <class T> using Lattice6 = Lattice<T,6>;
template <class T> using Lattice7 = Lattice<T,7>;
template <class T> using Lattice8 = Lattice<T,8>;
template <class T> using Lattice9 = Lattice<T,9>;
