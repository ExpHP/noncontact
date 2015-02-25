#include "../external-deps/catch.hpp"

#include <string>
#include <sstream>
#include <istream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <random>

#include "../points.hpp"

using namespace std;

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
Point<Basis> random_point (Basis basis, default_random_engine & rng);

template <>
Point<Cartesian> random_point <Cartesian> (Cartesian basis, default_random_engine & rng) {
	auto distribution = std::uniform_real_distribution<double> {-10., 10.};

	double x = distribution(rng);
	double y = distribution(rng);
	double z = distribution(rng);

	return make_point(x,y,z, basis);
}

template <>
Point<Cylindrical> random_point <Cylindrical> (Cylindrical basis, default_random_engine & rng) {
	double r = std::uniform_real_distribution<double> {0., 10.} (rng);
	double p = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (rng);
	double z = std::uniform_real_distribution<double> {-10., 10.} (rng);

	return make_point(r,p,z,basis);
}

template <>
Point<Spherical> random_point <Spherical> (Spherical basis, default_random_engine & rng) {
	double r = std::uniform_real_distribution<double> {0., 10.} (rng);
	double t = std::uniform_real_distribution<double> {0., 8.*atan(1.)} (rng);
	double p = std::uniform_real_distribution<double> {0., 4.*atan(1.)} (rng);

	return make_point(r,t,p,basis);
}
//--------------------------------------

template <class Basis1, class Basis2>
void test_reversibility (Basis1 b1, Basis2 b2, int repeat, default_random_engine & rng) {
	while (repeat--) {
		auto original = random_point(b1, rng);
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
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine rng(seed);

	SECTION("Cartesian -> Cylindrical and back") {
		test_reversibility(Cartesian{}, Cylindrical{}, 5, rng);
	}
	SECTION("Cylindrical => Cartesian and back") {
		test_reversibility(Cylindrical{}, Cartesian{}, 5, rng);
	}
	SECTION("Cartesian => Spherical and back") {
		test_reversibility(Cartesian{}, Spherical{}, 5, rng);
	}
	SECTION("Spherical => Cartesian and back") {
		test_reversibility(Spherical{}, Cartesian{}, 5, rng);
	}
}
