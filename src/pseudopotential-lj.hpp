#pragma once

#include "external-deps/catch.hpp"

#include <vector>
#include <chrono>

#include <Eigen/Dense>

#include "potential-lj.hpp"
#include "lattice.hpp"

// FIXME: `using` in header = badness
using namespace Eigen;

template <class T>
class LJPseudoPotential {
	public:
		static LJPseudoPotential fit_to_data(Lattice3<T> potential)
		{
			LJPseudoPotential result{};

			// make coeff lattices in same shape and size as the potential

			// FIXME omg wall of initialization  (this API won't cut it)
			result.z0 = Lattice2<T>{potential.size_1(), potential.size_2()}
				.set_lower_coords(potential.lower_coord_1(), potential.lower_coord_2())
				.set_upper_coords(potential.upper_coord_1(), potential.upper_coord_2());
			result.coeff6 = Lattice2<T>{potential.size_1(), potential.size_2()}
				.set_lower_coords(potential.lower_coord_1(), potential.lower_coord_2())
				.set_upper_coords(potential.upper_coord_1(), potential.upper_coord_2());
			result.coeff12 = Lattice2<T>{potential.size_1(), potential.size_2()}
				.set_lower_coords(potential.lower_coord_1(), potential.lower_coord_2())
				.set_upper_coords(potential.upper_coord_1(), potential.upper_coord_2());


			// Collect z-coordinates of data set
			Array<T, Dynamic, 1> zarr(potential.size_3(), 1);
			for (std::size_t k=0; k < potential.size_3(); k++) {
				zarr(k) = potential.coord_at_3(k);
			}

			// Generate coefficient matrix
			Matrix<T, Dynamic, 2> xmat {potential.size_3(), 2};
			zarr = zarr * zarr * zarr; // compute z^3
			zarr = zarr * zarr;        // compute z^6
			xmat.col(0) = zarr;        // fill col 0 with z^6
			xmat.col(1) = zarr * zarr; // fill col 1 with z^12

			// Generate least squares system solver
			auto solver = xmat.colPivHouseholderQr();

			// Solve for a best fit along z at each (x,y)
			for (std::size_t i=0; i < potential.size_1(); i++) {
				for (std::size_t j=0; j < potential.size_2(); j++) {

					// Collect potential data at (x,y)
					Matrix<T, Dynamic, 1> pvec {potential.size_3(), 1};
					for (std::size_t k=0; k < potential.size_3(); k++) {
						pvec(k) = potential(i,j,k);
					}

					// FIXME: a third parameter is needed (the z=0 point)

					// Solve the least squares system
					Matrix<T, 2, 1> soln = solver.solve(pvec);

					// Record
					result.z0(i,j)      = 0; // FIXME compute this proper
					result.coeff6(i,j)  = soln(0,0);
					result.coeff12(i,j) = soln(1,0); 
				}
			}

			return result;
		}

		T value_at(T x, T y, T z) {
			// do nearest neighbor for x and y <---- HORRIBLE IDEA  FIXME FIXME FIXME
			int i = nearest_index_x(x);
			int j = nearest_index_y(y);
			T r   = z - z0(i,j);
			T c6  = coeff6(i,j);
			T c12 = coeff12(i,j);

			return c6 * pow(r,6) + c12 * pow(r,12);
		}

		T force_z_derivative_at(T x, T y, T z) {
			// do nearest neighbor for x and y <---- HORRIBLE IDEA  FIXME FIXME FIXME
			int i = nearest_index_x(x);
			int j = nearest_index_y(y);
			T r   = z - z0(i,j);
			T c6  = coeff6(i,j);
			T c12 = coeff12(i,j);

			// expression obtained by le maths
			T result = 0;
			result += (36*4*26*27) * pow(r, -28) * c12 * c12;
			result += (36*4*20*21) * pow(r, -22) * c12 * c6;
			result += (36*1*14*15) * pow(r, -15) * c6  * c6;

			// TODO: This final expression should multiplied by some scalar constant related to
			//       polarization of the tip. Not sure what determines this constant, not sure
			//       where the best place is to account for it, not sure whether or not is is
			//       important enough to warrant further muddying the responsibilities of this class >_>
			return result;
		}

	private:
		// returns lattice indices of nearest point on lattice
		int nearest_index_x (T x) {
			return int((x - coeff6.lower_coord_1())/coeff6.coord_step_1() + 0.5);
		}
		int nearest_index_y (T y) {
			return int((y - coeff6.lower_coord_2())/coeff6.coord_step_2() + 0.5);
		}

		Lattice2<T> z0;
		Lattice2<T> coeff6;
		Lattice2<T> coeff12;
};

template <typename R>
void test_fit_to_lj (R & rng, std::array<size_t,3> dims, std::array<double,3> pos)
{
	double emin_true = 0.7;
	double rmin_true = 0.23;

	// single particle LJ Potential, with particle at x=0.2, y=0.3
	LJPotential<double> lj;
	//lj.add_particle(particle_x, particle_y, 0., emin_true, rmin_true);
	lj.add_particle(pos[0], pos[1], pos[2], emin_true, rmin_true);
	
	// populate potential data
	auto data = Lattice3<double> {dims[0], dims[1], dims[2]} // 11x11 so x,y coords are 0, 0.1, 0.2, ..., 1
		.set_lower_coords(0., 0., 0.5)
		.set_upper_coords(1., 1., 0.9)
	;
	for (std::size_t i=0; i < data.size_1(); i++) {
		for (std::size_t j=0; j < data.size_2(); j++) {
			for (std::size_t k=0; k < data.size_3(); k++) {	
				double x = data.coord_at_1(i);
				double y = data.coord_at_2(j);
				double z = data.coord_at_3(k);
				data(i, j, k) = lj.value_at(x, y, z);
			}
		}
	}

	// fit the data
	LJPseudoPotential<double> fit = LJPseudoPotential<double>::fit_to_data(data);

	int niter = 10;
	while (niter--) {
		double sample_z = std::uniform_real_distribution<double> {.5, .9} (rng);
		REQUIRE(
			lj.value_at(pos[0], pos[1], sample_z) ==
			Approx(fit.value_at(pos[0], pos[1], sample_z))
		);
	}
}

// FIXME PLACEHOLDER TO ENSURE COMPILATION
TEST_CASE("pseudopotential-lj") {

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine rng(seed);

	SECTION("LJ Pseudopotential from an actual LJ Potential") {

		SECTION("From a particle at z=0") {
			// for this case, a polynomial regression will do
			test_fit_to_lj (rng, {11, 11, 25}, {0.2, 0.3, 0.0});
		}
		SECTION("From a particle at nonzero z") {
			// this case is a bit less trivial
			test_fit_to_lj (rng, {11, 11, 25}, {0.2, 0.3, 0.1}); // FAILS (low priority)
		}
	}
}
