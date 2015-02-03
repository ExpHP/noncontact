#pragma once

#include <functional>

#include "external-deps/catch.hpp"


// Behold SFINAE, C++'s sorry excuse for compile-time type constraints.
template<
  typename F,        // a callable (function pointer, lambda, etc.)
  typename T,        // return type of F, and type of its first parameter (the integration variable)
  typename... Args,  // types of additional args of F

  // constraint:  F is a function that takes (T, Args...) and returns T
  typename = typename std::enable_if<std::is_convertible<
   decltype(std::declval<F>()(std::declval<T>(), std::declval<Args>()...)), T>::value>::type
>
T integrate_simpson(F callback, T a, T b, unsigned regions, Args... args)
{
	T sum = 0.;

	T h = (b - a) / (2*regions);

	// endpoints of full interval
	sum += callback(a, args...) + callback(b, args...);

	// midpoints of each region
	for (unsigned i=0; i<regions; i++)
		sum += 4*callback(a + (2*i+1)*h, args...);

	// shared endpoints between each region
	for (unsigned i=1; i<regions; i++)
		sum += 2*callback(a + (2*i+0)*h, args...);

	return sum * h / 3.;
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
