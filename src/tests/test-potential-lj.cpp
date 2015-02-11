#include "../external-deps/catch.hpp"

#include <iostream>

#include "../potential-lj.hpp"

using namespace std;

TEST_CASE("Parameters of LJ potential correspond to minimum") {
	double emin = 32.;
	double rmin = 5.;

	LJPotential<double> p;
	p.add_particle(10., 10., 10., emin, rmin);

	// one point on the minimum would be at (13,14,10)
	SECTION("-1 * energy_unit occurs at minimum") {
		REQUIRE(p.value_at(13., 14., 10.) == Approx(-emin));
	}

	SECTION("points around minimum have greater potential") {
		REQUIRE(p.value_at(12., 14., 10.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(14., 14., 10.) > p.value_at(13., 14., 10.));

		REQUIRE(p.value_at(13., 13., 10.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(13., 15., 10.) > p.value_at(13., 14., 10.));

		REQUIRE(p.value_at(13., 14.,  9.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(13., 14., 11.) > p.value_at(13., 14., 10.));
	}
}

TEST_CASE("Behavior of LJ Potential beyond minimum") {
	double emin = 32.;
	double rmin = 3.;

	LJPotential<double> p;
	p.add_particle(10., 10., 10., emin, rmin);

	// sample points at increasing distance from particle, in various directions
	double xs[4] = { 12., 13., 15., 1000. };
	double ys[4] = { 12.,  7.,  5., 1000. };
	double zs[4] = { 12.,  7., 15., 1000. };
	SECTION("All values after minimum are negative") {
		for (size_t i=0u; i<4u; i++) {
			REQUIRE(
				p.value_at(xs[i], ys[i], zs[i]) <
				0.
			);
		}
	}
	SECTION("Potential tends to zero at large distances") {
		for (size_t i=0u; i<3u; i++) {
			REQUIRE(
				fabs(p.value_at(xs[i],   ys[i],   zs[i]  )) >
				fabs(p.value_at(xs[i+1], ys[i+1], zs[i+1]))
			);
		}
	}
}
