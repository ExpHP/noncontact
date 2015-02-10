#include "../external-deps/catch.hpp"

#include "../numcomp.hpp"

TEST_CASE("Simpson's rule") {

	// functions to integrate
	auto x_cubed        = [] (double x) { return x*x*x;   };
	auto x_to_da_fourth = [] (double x) { return x*x*x*x; };
	auto constant       = [] (double x, double c) { return c; };

	SECTION("Test a known value") {
		SECTION("Integrating forwards") {
			REQUIRE(integrate_simpson(x_cubed, 0., 10., 100000) == Approx(2500));
		}
		SECTION("Integrating backwards") {
			REQUIRE(integrate_simpson(x_cubed, 10., 0., 100000) == Approx(-2500));
		}
	}

	SECTION("Confirm analytical error") {
		SECTION("Simpson's rule is exact for third order polynomials") {
			REQUIRE(
				Approx(integrate_simpson(x_cubed, 0., 10., 1)) // 1 region - just as accurate as
				 == integrate_simpson(x_cubed, 0., 10., 10000) // many regions
			);
		}
		SECTION("Simpson's rule is not exact for fourth order polynomials") {
			REQUIRE(
				Approx(integrate_simpson(x_to_da_fourth, 0., 10., 1)) // 1 region - less accurate than
				 != integrate_simpson(x_to_da_fourth, 0., 10., 10000) // many regions
			);
		}
	}

	SECTION("Try a function that has an additional arg") {
		double first  = integrate_simpson(constant, 5., 10., 10000, 32.); // integrate 32 from 5 to 10
		double second = integrate_simpson(constant, 5., 10., 10000, 42.); // integrate 42 from 5 to 10
		REQUIRE( (second - first) == Approx(50.));
	}
}


TEST_CASE("5-point stencil") {

	// functions to derive
	auto x_4 = [] (double x) { return pow(x,4); };
	auto x_5 = [] (double x) { return pow(x,5); };
	auto constant = [] (double x, double c) { return c; };

	SECTION("Known derivatives") {
		// derivative of a constant
		REQUIRE(differentiate_5point(constant, 32., 2., 12.) == Approx(0.));

		// derivative of x^5
		REQUIRE(differentiate_5point(x_5, 12., 0.00001) == Approx(5. * x_4(12.)));
	}

	SECTION("Confirm analytical error") {
		SECTION("5-point stencil is exact for fourth order") {
			REQUIRE(
				Approx(differentiate_5point(x_4, 2., 10.)) // large step
				 == differentiate_5point(x_4, 2., 0.0001)  // small step
			);
		}
		SECTION("5-point stencil is not exact for fifth order") {
			REQUIRE(
				Approx(differentiate_5point(x_5, 2., 10.)) // large step
				 != differentiate_5point(x_5, 2., 0.0001)  // small step
			);
		}
	}

}


TEST_CASE("Convergence") {

	SECTION("A simple case") {
		// limit of 1 as x goes to zero
		auto unsafe = [] (double x) { return x/x; };
		REQUIRE(std::isnan(unsafe(0.)));   // double-check that unsafe is poorly defined at x=0.
		CHECK_NOTHROW(converge(unsafe, 2., 0.5, 1E-10, -1) == Approx(1.)); // limit at x=0
	}

	SECTION("To infinity") {
		// limit of 1/x as x -> infinity
		auto reciprocal = [] (double x) { return 1./x; };
		CHECK(converge(reciprocal, 1., 2., 1E-10, -1) == Approx(0.));
	}

	SECTION("Using for adaptive integration") {

		// Function to integrate, which Simpson method cannot do exactly
		auto x_4 = [] (double x) { return pow(x, 5.); };
		auto the_func = [&x_4] (unsigned n) { return integrate_simpson(x_4, 0., 10., n); };

		SECTION("Should fail if given insufficient iterations to converge") {
			// triple the number of regions each step, for 2 steps
			REQUIRE_THROWS(converge(the_func, 1u, 3u, 1E-8, 2));
		}
		SECTION("Should succeed if given sufficient iterations to converge") {
			// triple the number of regions each step, for up to 100 steps
			REQUIRE_NOTHROW(converge(the_func, 1u, 3u, 1E-8, 100));
		}
	}
}
