#include "../external-deps/catch.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <climits>
#include <cstdlib>
#include <random>
#include <ctime>

#include "../points.hpp"

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

template <class B>
void require_approx_eq (PointCollection<B> a, PointCollection<B> b) {
	REQUIRE( a.size() == b.size() );
	for (std::size_t i=0; i<a.size(); ++i)
		require_approx_eq( a.point(i), b.point(i) );
}

//--------------------------------------
template <class Basis>
Point<Basis> random_point (Basis basis);

template <class Basis>
Basis random_basis ();

template <>
auto random_point (Cartesian basis) -> Point<decltype(basis)> {
	auto distribution = std::uniform_real_distribution<double> {-10., 10.};

	double x = distribution(RNG);
	double y = distribution(RNG);
	double z = distribution(RNG);

	return make_point(x,y,z, basis);
}

template <>
auto random_point (Cylindrical basis) -> Point<decltype(basis)> {
	double r = std::uniform_real_distribution<double> {0., 10.} (RNG);
	double p = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (RNG);
	double z = std::uniform_real_distribution<double> {-10., 10.} (RNG);

	return make_point(r,p,z,basis);
}

template <>
auto random_point (Spherical basis) -> Point<decltype(basis)> {
	double r = std::uniform_real_distribution<double> {0., 10.} (RNG);
	double t = std::uniform_real_distribution<double> {0., 4.*atan(1.)} (RNG);
	double p = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (RNG);

	return make_point(r,t,p,basis);
}

template <>
auto random_point (ScaledCartesian basis) -> Point<decltype(basis)> {
	// Just make a random cartesian point and use its coords
	auto src = random_point(Cartesian{});
	return tag_point(src.as_raw(),basis);
}

template <>
auto random_point (VectorBasis basis) -> Point<decltype(basis)> {
	// Just make a random cartesian point and use its coords
	auto src = random_point(Cartesian{});
	return tag_point(src.as_raw(),basis);
}

//--------------------------------------
template <class Basis>
PointCollection<Basis> random_point_collection (Basis basis, std::size_t n) {
	PointCollection<Basis> collection(basis);
	for (std::size_t i=0; i<n; ++i)
		collection.emplace_back(random_point(basis).as_raw());
	return collection;
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
	SECTION("Cartesian => Spherical and back") {
		test_reversibility(Cartesian{}, Spherical{}, 5);
	}

	// As Points expose mutable references to their data, Cylindrical & Spherical
	//   points can be given any sort of arbitrary angles, and so there's no decent
	//   way to impose invariants on the range of the angles.
	// Thus, let's skip testing Cylindrical/Spherical to Cartesian and back,
	//   as the success or failure of such a test would not be meaningful :(

	SECTION("To Spherical/Cylindrical and back with 0 radius") {
		// Mathematically speaking, this is poorly defined.
		// _Practically_ speaking, we sure as hell better get _something_!
		auto original = make_point(0.,0.,0., Cartesian{});
		auto viaCylindrical = original.transform(Cylindrical{}).transform(Cartesian{});
		auto viaSpherical   = original.transform(Spherical{}).transform(Cartesian{});

		require_approx_eq(original, viaCylindrical);
		require_approx_eq(original, viaSpherical);
	}

	// Some fixed arbitrary scale
	ScaledCartesian scaledbasis {0.45};
	SECTION("ScaledCartesian => Cartesian and back") {
		test_reversibility(scaledbasis, Cartesian{}, 5);
	}
	SECTION("Cartesian => ScaledCartesian and back") {
		test_reversibility(Cartesian{}, scaledbasis, 5);
	}

	// Some fixed arbitrary vector basis
	VectorBasis vectorbasis {{
		{+9.644, +0.657,  +15.392 },
		{-5.955, +17.859, -1.445  },
		{-8.205, -10.555, +19.556 }
	}};
	SECTION("VectorBasis => Cartesian and back") {
		test_reversibility(vectorbasis, Cartesian{}, 5);
	}
	SECTION("Cartesian => VectorBasis and back") {
		test_reversibility(Cartesian{}, vectorbasis, 5);
	}

}

//--------------------------------------

TEST_CASE("Simple VectorBasis tests") {
	// A simple vector basis which permutes axes
	VectorBasis vectorbasis {{
		{0., 1. ,0. },
		{0., 0. ,1. },
		{1., 0., 0. }
	}};

	SECTION("Cartesian => Simple VectorBasis") {
		require_approx_eq(
			make_point(4.,5.,6.,Cartesian{}).transform(vectorbasis),
			make_point(6.,4.,5.,vectorbasis)
		);
	}
	SECTION("Simple VectorBasis => Cartesian") {
		require_approx_eq(
			make_point(4.,5.,6.,vectorbasis).transform(Cartesian{}),
			make_point(5.,6.,4.,Cartesian{})
		);
	}
}

//--------------------------------------

// Conversions from a coordinate system to itself
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
template<> RawPoint transform (const RawPoint & point, const Cartesian &, const TestBasis &)
{
	return { point.second(), point.third(), point.first() };
}

// TestBasis -> Cartesian
template<> RawPoint transform (const RawPoint & point, const TestBasis &, const Cartesian &)
{
	return { point.third(), point.first(), point.second() };
}

// TestBasis -> Spherical
template<> RawPoint transform (const RawPoint & point, const TestBasis &, const Spherical &)
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

//--------------------------------------



TEST_CASE("Test that Point Collections are not horribly broken") {
	auto collection = make_point_collection(Cartesian{});

	collection.emplace_back(RawPoint{{20.,30.,40.}});
	collection.emplace_back(RawPoint{{10.,2.3,4.}});
	collection.emplace_back(RawPoint{{-2.,0.,1.}});
	REQUIRE( collection.size() == 3 );

	REQUIRE( collection.point(1).second() == Approx(2.3) );

	// modify through operator[]
	collection[1][1] = 4.5;
	REQUIRE( collection.point(1).second() == Approx(4.5) );

	// raw() and point() should have the same behavior (both modify, or neither modify).
	// Current contract is that neither permit modification. 

	collection.point(1).second() = 1.2;
	REQUIRE( collection.point(1).second() == Approx(4.5) );

	collection.raw(1)[1] = 2.0;
	REQUIRE( collection.point(1).second() == Approx(4.5) );
}

TEST_CASE("Conversions on point collections") {

	SECTION("Test fallback to point conversion") {
		auto cartesian = make_point_collection(Cartesian{});
		cartesian.emplace_back(RawPoint{{4.,5.,6.}});
		cartesian.emplace_back(RawPoint{{10.,100.,1000.}});
		cartesian.emplace_back(RawPoint{{3.,2.,1.}});

		auto expected = make_point_collection(TestBasis{});
		expected.emplace_back(RawPoint{{5.,6.,4.}});
		expected.emplace_back(RawPoint{{100.,1000.,10.}});
		expected.emplace_back(RawPoint{{2.,1.,3.}});

		require_approx_eq(cartesian.transform(TestBasis{}), expected);
	}

	// TODO: Test an actual, non-fallback conversion

	SECTION("Test fallback to point conversion, when it falls back to Cartesian") { // words
		auto cartesian = random_point_collection(Cartesian{}, 3);
		auto testbasis = cartesian.transform(TestBasis{});

		auto fromCartesian = cartesian.transform(Cylindrical{});
		auto fromTestbasis1 = testbasis.transform(Cylindrical{}); // member function
		auto fromTestbasis2 = transform(testbasis,Cylindrical{}); // free function

		require_approx_eq(fromTestbasis1, fromCartesian);
		require_approx_eq(fromTestbasis2, fromCartesian);
	}

	SECTION("Test fallback to point conversion, when Cartesian fallback isn't needed") { // arghfffbl
		// let's just make a collection of one point
		auto point = make_point(0.,0.,0., TestBasis{});
		auto points = PointCollection<TestBasis>(TestBasis{}); // lazy
		points.emplace_back(point.as_raw());

		REQUIRE_THROWS_AS(points.transform(Spherical{}),  float); // member function
		REQUIRE_THROWS_AS(transform(points, Spherical{}), float); // free function
	}

}
