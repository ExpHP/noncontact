#pragma once

#include "external-deps/catch.hpp"

#include "potential-lj.hpp"
#include "numcomp.hpp"

// Forward declarations
template <class P> double horrible_unsightly_method_of_integration(const P & potential, double x, double y, double z);

class CaciucMethod
{
public:

	CaciucMethod()
	: some_factor(1.)
	{ }

	template <class P>
	inline double frequency_shift_at(const P & potential, double x, double y, double z)
	{
		return this->some_factor * horrible_unsightly_method_of_integration(potential, x, y, z);
	}

private:
	// placeholder for the constant overall prefactor in the frequency shift,
	//  which should later be replaced by something legitimate (TODO)
	double some_factor; 
};

template <class P>
double horrible_unsightly_method_of_integration(const P & potential, double x, double y, double z)
{
	// One step at a time so that I (probably) can't possibly mess it up

	// Express the integrand
	auto p_of_z    = [=] (double zt) { return potential.value_at(x, y, zt); };
	auto p_of_u    = [=] (double u) { return p_of_z(z + pow(log(u), 2)); };
	auto dp_du     = [=] (double u) { return differentiate_5point(p_of_u, u, 0.01*u); };
	auto integrand = [=] (double u) { return dp_du(u) / log(u); };

	// Perform the integral, bringing the upper bound closer and closer to 1 until convergence
//	auto approx_integral = [&,integrand] (double h) { return integrate_simpson(integrand, 0., 1.-h, 10000u); };
	auto approx_integral = [&,integrand] (double h) { return integrate_simpson(integrand, h, 1.-h, 10u); };
	return -1. * converge(approx_integral, 0.125, 0.5, 1E-3, 2000u);
}


TEST_CASE("Caciuc method") {

	CaciucMethod cm{};

	SECTION("Near a single particle") {
		LJPotential<double> pot{};
		pot.add_particle(0.5, 0.5, 0.25, 1., 0.1);
		pot.add_particle(0.5, 0.5, 0.25, 1., 0.1);

		SECTION("Frequency shift is... ah... erm... finite") {
			REQUIRE_NOTHROW( cm.frequency_shift_at(pot, 0.5, 0.5, 0.5) );
			double df = cm.frequency_shift_at(pot, 0.5, 0.5, 0.35);

			std::cout << df << std::endl;
			REQUIRE(!std::isnan(df));
			REQUIRE(df != Approx(0.));
			REQUIRE(std::isfinite(df));
		}
	}

}
