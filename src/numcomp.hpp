#pragma once

#include <functional>
#include <cmath>

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
R converge(F func, T init, T factor, R tol, int maxiter, Args... args)
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
