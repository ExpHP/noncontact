#pragma once

#include <functional>

#include "external-deps/catch.hpp"

// A SFINAE construct that constrains F to be a function of (T, Args...) -> T,
//  which is the form of function that most methods here are intended to work with.
#define REQUIRE_MATH_FUNC(F,T,Args)                                                   \
	typename = typename std::enable_if<                                               \
		std::is_convertible<                                                          \
			decltype(std::declval<F>()(std::declval<T>(), std::declval<Args>()...)),  \
			T                                                                         \
		>::value                                                                      \
	>::type


template<
  typename F, typename T, typename... Args,
  REQUIRE_MATH_FUNC(F, T, Args)
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
  REQUIRE_MATH_FUNC(F, T, Args)
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
