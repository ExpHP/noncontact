#include "../external-deps/catch.hpp"

#include "../pseudopotential-lj.hpp"
#include "../potential-lj.hpp"

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
	auto data = Lattice3<double>(dims) // 11x11 so x,y coords are 0, 0.1, 0.2, ..., 1
		.set_lower_coords(0., 0., 0.5)
		.set_upper_coords(1., 1., 0.9)
	;
	for (std::size_t i=0; i < data.axis_size(1); i++) {
		for (std::size_t j=0; j < data.axis_size(2); j++) {
			for (std::size_t k=0; k < data.axis_size(3); k++) {
				double x = data.coord(1,i);
				double y = data.coord(2,j);
				double z = data.coord(3,k);
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
