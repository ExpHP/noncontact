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
	auto converted = original.transform(basis);

	require_approx_eq(original, converted);
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
template<> RawPoint transform (const RawPoint &, const TestBasis &, const Spherical &)
{
	throw float(1234); // something unusual and easily detected
}

// Note: Here is an explicit instantiation of the function that is supposed to use the below
//         definition of transform_range. If uncommented, it will instantiate the method early
//         and cause it to bind to the default implementation of transform_range instead,
//         breaking one of the tests.
//      (if placed below the method, it will have no effect)
//template PointCollection<Cartesian> PointCollection<TestBasis>::transform<Cartesian>(Cartesian) const;

// TestBasis -> Cartesian (range)
template <class InIter, class OutIter>
void transform_range (InIter, InIter, OutIter, const TestBasis &, const Cartesian &)
{
	throw int(92); // something unusual and easily detected
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
		auto fromTestbasis = testbasis.transform(Cylindrical{});

		require_approx_eq(fromTestbasis, fromCartesian);
	}

	SECTION("Test that fallback isn't used when not needed") {
		auto point = make_point(0.,0.,0., TestBasis{});

		// TestBasis => Spherical is implemented, and throws a float
		//  (something which the fallback certainly does not do!)
		REQUIRE_THROWS_AS(point.transform(Spherical{}),  float);
	}

}

//--------------------------------------



TEST_CASE("Test that Point Collections are not horribly broken") {
	auto collection = make_point_collection(Cartesian{});

	collection.emplace_back(20.,30.,40.);
	collection.emplace_back(10.,2.3,4.);
	collection.emplace_back(-2.,0.,1.);
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

	SECTION("Test explicitly implemented collection conversions") {
		auto testbasis = make_point_collection(TestBasis{});
		testbasis.emplace_back(5.,6.,4.);
		testbasis.emplace_back(100.,1000.,10.);
		testbasis.emplace_back(2.,1.,3.);

		REQUIRE_THROWS_AS(testbasis.transform(Cartesian{}), int);
	}

	SECTION("Test fallback to point conversion") {
		auto cartesian = make_point_collection(Cartesian{});
		cartesian.emplace_back(4.,5.,6.);
		cartesian.emplace_back(10.,100.,1000.);
		cartesian.emplace_back(3.,2.,1.);

		auto expected = make_point_collection(TestBasis{});
		expected.emplace_back(5.,6.,4.);
		expected.emplace_back(100.,1000.,10.);
		expected.emplace_back(2.,1.,3.);

		require_approx_eq(cartesian.transform(TestBasis{}), expected);
	}

	// TODO: Test an actual, non-fallback conversion

	SECTION("Test fallback to point conversion, when it falls back to Cartesian") { // words
		auto cartesian = random_point_collection(Cartesian{}, 3);
		auto testbasis = cartesian.transform(TestBasis{});

		auto fromCartesian = cartesian.transform(Cylindrical{});
		auto fromTestbasis = testbasis.transform(Cylindrical{});

		require_approx_eq(fromTestbasis, fromCartesian);
	}

	SECTION("Test fallback to point conversion, when Cartesian fallback isn't needed") { // arghfffbl
		// let's just make a collection of one point
		auto point = make_point(0.,0.,0., TestBasis{});
		auto points = PointCollection<TestBasis>(TestBasis{}); // lazy
		points.emplace_back(point.as_raw());

		REQUIRE_THROWS_AS(points.transform(Spherical{}), float);
	}

}

// performs various things with a dynamic basis and another basis to see if they compile
template <class Basis>
void dynamic_basis_compile_test(AnyBasis any, Basis other) {
	RawPoint p {0.2, 0.4, 0.6};
	RawPointCollection v;
	RawPointCollection out(1);
	v.push_back(p);

	transform(p, any, other);
	transform(p, other, any);
	transform_range(v.begin(), v.end(), out.begin(), any, other);
	transform_range(v.begin(), v.end(), out.begin(), other, any);
}

//  converts a point from one basis to another statically and dynamically to see if they match
template <class FromBasis, class ToBasis>
void dynamic_basis_consistency_test(FromBasis from, ToBasis to) {
	auto ptRaw     = random_point(from).as_raw();
	Point<FromBasis> ptStatic  = tag_point(ptRaw, from);
	Point<AnyBasis>  ptDynamic = tag_point(ptRaw, AnyBasis(from));

	PointCollection<FromBasis> listStatic  = random_point_collection(from, 3);
	PointCollection<AnyBasis>  listDynamic {listStatic.as_raw(), AnyBasis(from)};

	ToBasis  toStatic  = to;
	AnyBasis toDynamic = AnyBasis(to);

	// static type to static type
	auto ptExpected   = ptStatic.transform(toStatic);
	auto listExpected = listStatic.transform(toStatic);

	// The expectation is that AnyBasis calls the exact same implementation as would
	//  have been called statically.
	// Thus, these tests use == comparison.
	REQUIRE( ptStatic.transform(toDynamic).as_raw()  == ptExpected.as_raw() );
	REQUIRE( ptDynamic.transform(toStatic).as_raw()  == ptExpected.as_raw() );
	REQUIRE( ptDynamic.transform(toDynamic).as_raw() == ptExpected.as_raw() );
	REQUIRE( listStatic.transform(toDynamic).as_raw()  == listExpected.as_raw() );
	REQUIRE( listDynamic.transform(toStatic).as_raw()  == listExpected.as_raw() );
	REQUIRE( listDynamic.transform(toDynamic).as_raw() == listExpected.as_raw() );

	// Erm... let's make sure that list comparison is actually checking the elements
	listExpected[2].first() = 0;
	REQUIRE( listDynamic.transform(toStatic).as_raw() != listExpected.as_raw() );
}

TEST_CASE("Dynamic Basis") {
	SECTION("Compilation test: Create from various types") {
		// Cause instantiation of some of AnyBasis's templated inner classes
		AnyBasis anyCart(Cartesian{});
		AnyBasis anySpher(Spherical{});
		AnyBasis anyCylind(Cylindrical{});
		AnyBasis anyScaled(ScaledCartesian{0.5});

		SECTION("Compilation test: Try some transformations") {
			// Between a dynamic basis and Cartesian
			dynamic_basis_compile_test(anySpher, Cartesian{});

			// Between a dynamic basis that is cartesian and Cartesian
			dynamic_basis_compile_test(anyCart, Cartesian{});

			// Between a dynamic basis and something not Cartesian
			dynamic_basis_compile_test(anyScaled, Spherical{});

			// Between two dynamic bases
			dynamic_basis_compile_test(anySpher, anyCylind);
		}
	}

	SECTION("Dynamic basis conversions are consistent with static conversions") {
		dynamic_basis_consistency_test(Cylindrical{}, Cartesian{});   // to cart
		dynamic_basis_consistency_test(Cartesian{},   Cylindrical{}); // from cart
		dynamic_basis_consistency_test(Cartesian{},   Cartesian{});   // cart to cart
		dynamic_basis_consistency_test(Spherical{},   Cylindrical{}); // non-cart to non-cart
	}
}
