#pragma once

#include "external-deps/catch.hpp"

#include <vector>

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
					result.coeff6(i,j)  = soln(0,0);
					result.coeff12(i,j) = soln(1,0); 
				}
			}

			return result;
		}

		T value_at(T x, T y, T z) {
			// do nearest neighbor for x and y <---- HORRIBLE IDEA  FIXME FIXME FIXME
			int i = int((x - coeff6.lower_coord_1())/coeff6.coord_step_1() + 0.5);
			int j = int((y - coeff6.lower_coord_2())/coeff6.coord_step_2() + 0.5);
			T c6  = coeff6(i,j);
			T c12 = coeff12(i,j);

			return c6 * pow(z,6) + c12 * pow(z,12);
		}

	private:
		Lattice2<T> coeff6;
		Lattice2<T> coeff12;
};

TEST_CASE("pseudopotential-lj") {
	std::default_random_engine rng;

	SECTION("LJ Pseudopotential from an actual LJ Potential") {

		SECTION("From a particle at z=0") {

			double particle_x = 0.2;
			double particle_y = 0.3;
			double emin_true = 0.7;
			double rmin_true = 0.23;

			// single particle LJ Potential, with particle at x=0.2, y=0.3
			LJPotential<double> lj;
			lj.add_particle(particle_x, particle_y, 0., emin_true, rmin_true);
			
			// populate potential data
			auto data = Lattice3<double> {11, 11, 2} // 11x11 so x,y coords are 0, 0.1, 0.2, ..., 1
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

			SECTION("The fitted potential above the particle should be fairly accurate") {
				int niter = 10;
				while (niter--) {
					double sample_z = std::uniform_real_distribution<double> {.5, .9} (rng);
					REQUIRE(
						lj.value_at(particle_x, particle_y, sample_z) ==
						Approx(fit.value_at(particle_x, particle_y, sample_z))
					);
				}
			}
		}
	}
}
