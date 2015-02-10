#include "../external-deps/catch.hpp"

#include "../potential-lj.hpp"

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
