#include "../external-deps/catch.hpp"

#include "../lattice.hpp"

TEST_CASE("Lattice coordinates") {
	Lattice3<double> m(5u, 9u, 3u);
	m.set_lower_coords(0.2,4.,-3.);
	m.set_upper_coords(0.6,5.,-1.);

	SECTION("Coords at boundaries") {
		for (int d=0; d<m.num_dims(); ++d) {
			REQUIRE( m.coord_n(d,0            ) == Approx(m.lower_coord_n(d)) );
			REQUIRE( m.coord_n(d,m.size_n(d)-1) == Approx(m.upper_coord_n(d)) );
		}
	}

	SECTION("Steps") {
		REQUIRE( m.coord_step_0() == Approx(0.1)   );
		REQUIRE( m.coord_step_1() == Approx(0.125) );
		REQUIRE( m.coord_step_2() == Approx(1.0)   );
	}

	SECTION("Various coords") {
		REQUIRE( m.coord_0(2) == Approx(0.4)   );
		REQUIRE( m.coord_1(3) == Approx(4.375) );
		REQUIRE( m.coord_2(1) == Approx(-2.)   );
	}

	SECTION("Coords are mutable") {
		m.set_lower_coords(0.2, 1., -3.); // Change ymin

		REQUIRE( m.coord_0(2) == Approx(0.4) ); // unaffected
		REQUIRE( m.coord_1(3) == Approx(2.5) ); // changed
		REQUIRE( m.coord_2(1) == Approx(-2.) ); // unaffected
	}

	SECTION("Behavior of reversed coordinate ranges") {
		m.set_lower_coords(1.0, 4., -3.); // now xmin > xmax

		// Behavior is defined; coords will decrease linearly
		REQUIRE( m.lower_coord_0() > m.upper_coord_0() );
		REQUIRE( m.coord_step_0() == Approx(-0.1) );
		REQUIRE( m.coord_0(1) == Approx(0.9) );
	}
}

TEST_CASE("Lattice dimensions") {
	Lattice3<double> m(5, 9, 3);

	SECTION("Various aspects") {
		REQUIRE( m.size() == 5*9*3 );

		REQUIRE( m.size_0() == 5 );
		REQUIRE( m.size_1() == 9 );
		REQUIRE( m.size_2() == 3 );

		REQUIRE( m.stride_0() == 3*9 );
		REQUIRE( m.stride_1() == 3   );
		REQUIRE( m.stride_2() == 1   );
	}
}

TEST_CASE("Making lattices derived from others") {
	Lattice<double,3> m(12, 10, 15);
	m.set_lower_coords(0.5, 1.7, 0.1);
	m.set_upper_coords(5.1, 9.2, 8.6);
	SECTION("Invoking make_sub_lattice with 0, 1, ..., D-1 is equivalent to make_similar_lattice") {
		REQUIRE( make_similar_lattice<double>(m) == make_sub_lattice<double>(m, 0, 1, 2) );
		REQUIRE( make_similar_lattice<double>(m) != make_sub_lattice<double>(m, 0, 2, 1) );
	}
}
