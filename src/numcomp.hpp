#pragma once

#include <functional>
#include <cmath>

#include "external-deps/catch.hpp"

// A SFINAE construct that constrains F to be a function of (T, Args...) returning R.
// Typically speaking, in this module, the methods vary the first parameter of the function
//  (the one of type T), while simply forwarding the others (of types Args...).
#define REQUIRE_MATH_FUNC(F,T,Args,R)                                                 \
	typename = typename std::enable_if<                                               \
		std::is_convertible<                                                          \
			decltype(std::declval<F>()(std::declval<T>(), std::declval<Args>()...)),  \
			R                                                                         \
		>::value                                                                      \
	>::type


template<
  typename F, typename T, typename... Args,
  REQUIRE_MATH_FUNC(F, T, Args, T)
>
T integrate_simpson(F func, T a, T b, unsigned regions, Args... args)
{
	T sum = 0.;

	T h = (b - a) / (2*regions);

	// endpoints of full interval
	sum += func(a, args...) + func(b, args...);

	// midpoints of each region
	for (unsigned i=0; i<regions; i++)
		sum += 4 * func(a + (2*i+1)*h, args...);

	// shared endpoints between each region
	for (unsigned i=1; i<regions; i++)
		sum += 2 * func(a + (2*i+0)*h, args...);

	return sum * h / 3.;
}


template<
  typename F, typename T, typename... Args,
  REQUIRE_MATH_FUNC(F, T, Args, T)
>
T differentiate_5point(F func, T x, T step, Args... args)
{
	T weights[4] = { 1., -8., 8., -1.};
	T points[4]  = {x-2.*step, x-step, x+step, x+2*step};

	T sum = 0.;
	for (size_t i=0; i<4; i++)
		sum += weights[i] * func(points[i], args...);

	return sum / (12. * step);
}


// Calls its input function with terms from a geometric series (defined by `init` and `factor`)
//  until it converges within a tolerance.
template<
  typename F, typename T, typename... Args, typename R,
  REQUIRE_MATH_FUNC(F, T, Args, R)
>
R converge(F func, T init, T factor, R tol, unsigned maxiter, Args... args)
{
	if (maxiter < 0)
		maxiter = std::numeric_limits<decltype(maxiter)>::max();

	T x = init;
	R prev = func(x, args...);

	x *= factor;
	R next = func(x, args...);

	while (fabs(prev - next) > tol) { // TODO: A abs_diff method with template specializations may be better
		if (maxiter-- == 0)
			throw std::runtime_error("Failed to converge"); // TODO include variables in message

		x *= factor;
		prev = next;
		next = func(x, args...);
	}

	return next;
}


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
			REQUIRE_THROWS(converge(the_func, 1, 3, 1E-8, 2));
		}
		SECTION("Should succeed if given sufficient iterations to converge") {
			// triple the number of regions each step, for up to 100 steps
			REQUIRE_NOTHROW(converge(the_func, 1, 3, 1E-8, 100));
		}
	}

}
