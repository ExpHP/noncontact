#include "../external-deps/catch.hpp"

#include "../lattice.hpp"

TEST_CASE("Lattice coordinates") {
	Lattice3<double> m(5u, 9u, 3u);
	m.set_lower_coords(0.2,4.,-3.);
	m.set_upper_coords(0.6,5.,-1.);

	SECTION("Coords at boundaries") {
		REQUIRE( m.coord(0,0) == Approx(m.lower_coord(0)) );
		REQUIRE( m.coord(1,0) == Approx(m.lower_coord(1)) );
		REQUIRE( m.coord(2,0) == Approx(m.lower_coord(2)) );

		REQUIRE( m.coord(0,4) == Approx(m.upper_coord(0)) );
		REQUIRE( m.coord(1,8) == Approx(m.upper_coord(1)) );
		REQUIRE( m.coord(2,2) == Approx(m.upper_coord(2)) );
	}

	SECTION("Steps") {
		REQUIRE( m.coord_step(0) == Approx(0.1)   );
		REQUIRE( m.coord_step(1) == Approx(0.125) );
		REQUIRE( m.coord_step(2) == Approx(1.0)   );
	}

	SECTION("Various coords") {
		REQUIRE( m.coord(0,2) == Approx(0.4)   );
		REQUIRE( m.coord(1,3) == Approx(4.375) );
		REQUIRE( m.coord(2,1) == Approx(-2.)   );
	}

	SECTION("Coords are mutable") {
		m.set_lower_coords(0.2, 1., -3.); // Change ymin

		REQUIRE( m.coord(0,2) == Approx(0.4) ); // unaffected
		REQUIRE( m.coord(1,3) == Approx(2.5) ); // changed
		REQUIRE( m.coord(2,1) == Approx(-2.) ); // unaffected
	}

	SECTION("Behavior of reversed coordinate ranges") {
		m.set_lower_coords(1.0, 4., -3.); // now xmin > xmax

		// Behavior is defined; coords will decrease linearly
		REQUIRE( m.lower_coord(0) > m.upper_coord(0) );
		REQUIRE( m.coord_step(0) == Approx(-0.1) );
		REQUIRE( m.coord(0,1) == Approx(0.9) );
	}
}

TEST_CASE("Lattice dimensions") {
	Lattice3<double> m(5, 9, 3);

	SECTION("Various aspects") {
		REQUIRE( m.size() == 5*9*3 );

		REQUIRE( m.axis_size(0) == 5 );
		REQUIRE( m.axis_size(1) == 9 );
		REQUIRE( m.axis_size(2) == 3 );

		REQUIRE( m.stride(0) == 3*9 );
		REQUIRE( m.stride(1) == 3   );
		REQUIRE( m.stride(2) == 1   );
	}

}
