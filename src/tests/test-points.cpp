#include "../external-deps/catch.hpp"

#include <string>
#include <sstream>
#include <istream>
#include <climits>
#include <cstdlib>
#include <random>
#include <ctime>

#include "../points.hpp"
#include "../points/spherical.hpp"
#include "../points/cylindrical.hpp"
#include "../points/vectorbasis.hpp"
#include "../points/scaled.hpp"

using namespace std;

default_random_engine RNG(time(0));

//--------------------------------------
template <class B>
void require_approx_eq (Point<B> a, Point<B> b) {
	// expand into three individual tests for easier debugging
	REQUIRE( a.first()  == Approx(b.first())  );
	REQUIRE( a.second() == Approx(b.second()) );
	REQUIRE( a.third()  == Approx(b.third())  );
}

//--------------------------------------
template <class Basis>
Point<Basis> random_point (Basis basis);

template <>
Point<Cartesian> random_point <Cartesian> (Cartesian basis) {
	auto distribution = std::uniform_real_distribution<double> {-10., 10.};

	double x = distribution(RNG);
	double y = distribution(RNG);
	double z = distribution(RNG);

	return make_point(x,y,z, basis);
}

template <>
Point<Cylindrical> random_point <Cylindrical> (Cylindrical basis) {
	double r = std::uniform_real_distribution<double> {0., 10.} (RNG);
	double p = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (RNG);
	double z = std::uniform_real_distribution<double> {-10., 10.} (RNG);

	return make_point(r,p,z,basis);
}

template <>
Point<Spherical> random_point <Spherical> (Spherical basis) {
	double r = std::uniform_real_distribution<double> {0., 10.} (RNG);
	double t = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (RNG);
	double p = std::uniform_real_distribution<double> {0., 4.*atan(1.)} (RNG);

	return make_point(r,t,p,basis);
}

//--------------------------------------

template <class Basis1, class Basis2>
void test_reversibility (Basis1 b1, Basis2 b2, int repeat) {
	while (repeat--) {
		auto original = random_point(b1);
		auto converted = original.transform(b2);
		auto recovered = converted.transform(b1);

		require_approx_eq(original, recovered);
	}
}

//--------------------------------------

TEST_CASE("Using make_point") {
	// expected typical usage
	auto point = make_point(2.4, 3.5, -12., Cartesian());

	// I'm not sure how implementing CRTP would affect the return type, so I'll put this
	//  here to let me know if it changes
	static_assert(std::is_same< decltype(point), Point<Cartesian> >::value, "Return type of make_point changed");

	REQUIRE(point.second() == 3.5);
}

TEST_CASE("Reversibility of built-in coordinate systems") {

	SECTION("Cartesian -> Cylindrical and back") {
		test_reversibility(Cartesian{}, Cylindrical{}, 5);
	}
	SECTION("Cylindrical => Cartesian and back") {
		test_reversibility(Cylindrical{}, Cartesian{}, 5);
	}
	SECTION("Cartesian => Spherical and back") {
		test_reversibility(Cartesian{}, Spherical{}, 5);
	}
	SECTION("Spherical => Cartesian and back") {
		test_reversibility(Spherical{}, Cartesian{}, 5);
	}

	SECTION("To Spherical/Cylindrical and back with 0 radius") {
		// Mathematically speaking, this is poorly defined.
		// _Practically_ speaking, we sure as hell better get _something_!
		auto original = make_point(0.,0.,0., Cartesian{});
		auto viaCylindrical = original.transform(Cylindrical{}).transform(Cartesian{});
		auto viaSpherical   = original.transform(Spherical{}).transform(Cartesian{});

		require_approx_eq(original, viaCylindrical);
		require_approx_eq(original, viaSpherical);
	}
}

//--------------------------------------

template <class Basis>
void test_trivial (Basis basis) {
	auto original = random_point(basis);
	auto converted1 = original.transform(basis);  // Member function
	auto converted2 = transform(original, basis); // Free function

	require_approx_eq(original, converted1);
	require_approx_eq(original, converted2);
}

TEST_CASE("Trivial conversions") {
	test_trivial(Cartesian{});
	test_trivial(Cylindrical{});
	test_trivial(Spherical{});
}

//--------------------------------------

// A Basis written exclusively for this test.
// It has an explicit conversion to Spherical, but not to Cylindrical.
// It is a Cartesian point with permuted axes (its basis vectors are y,z,x).
class TestBasis { };

// Cartesian -> TestBasis
template<> auto transform (const Point<Cartesian> & point, TestBasis basis) -> Point<decltype(basis)>
{
	return { point.second(), point.third(), point.first(), basis };
}

// TestBasis -> Cartesian
template<> auto transform (const Point<TestBasis> & point, Cartesian basis) -> Point<decltype(basis)>
{
	return { point.third(), point.first(), point.second(), basis };
}

// TestBasis -> Spherical
template<> auto transform (const Point<TestBasis> & point, Spherical basis) -> Point<decltype(basis)>
{
	throw float(1234); // something unusual and easily detected
}

TEST_CASE("Test fallback mechanism") {

	SECTION("Test the test basis") {
		// I mean, it would be lame if we only failed because TestBasis was broken
		auto original  = make_point(4.,5.,6., Cartesian{});
		auto expected  = make_point(5.,6.,4., TestBasis{});

		auto converted = original.transform(TestBasis{});  // convert to...
		auto recovered = converted.transform(Cartesian{}); // ...and from.

		require_approx_eq(original, recovered);
		require_approx_eq(converted, expected);
	}

	SECTION("Test fallback mechanism") {
		auto cartesian = random_point(Cartesian{});
		auto testbasis = cartesian.transform(TestBasis{});

		auto fromCartesian = cartesian.transform(Cylindrical{});

		// because TestBasis => Cylindrical is not implemented, this will use the fallback:
		auto fromTestbasis1 = testbasis.transform(Cylindrical{}); // member function
		auto fromTestbasis2 = transform(testbasis,Cylindrical{}); // free function

		require_approx_eq(fromTestbasis1, fromCartesian);
		require_approx_eq(fromTestbasis2, fromCartesian);
	}

	SECTION("Test that fallback isn't used when not needed") {
		auto point = make_point(0.,0.,0., TestBasis{});

		// TestBasis => Spherical is implemented, and throws a float
		//  (something which the fallback certainly does not do!)
		REQUIRE_THROWS_AS(point.transform(Spherical{}),  float); // member function
		REQUIRE_THROWS_AS(transform(point, Spherical{}), float); // free function
	}
}
