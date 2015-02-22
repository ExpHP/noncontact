#include "../external-deps/catch.hpp"

#include <string>
#include <sstream>
#include <istream>
#include <climits>
#include <cstdlib>

#include "../io/strict-parser.hpp"

using namespace std;

string test_word(const char * s) {
	istringstream ss(s);
	return StrictParser(ss).expect_word();
}

string test_line(const char * s) {
	istringstream ss(s);
	return StrictParser(ss).expect_line();
}

long test_integer(const char * s, bool skipws = true) {
	istringstream ss(s);

	if (!skipws)
		ss >> std::noskipws;

	return StrictParser(ss).expect_integer();
}

double test_float(const char * s, bool skipws = true) {
	istringstream ss(s);

	if (!skipws)
		ss >> std::noskipws;

	return StrictParser(ss).expect_float();
}

void test_end_of_content(const char * s) {
	istringstream ss(s);
	StrictParser(ss).expect_end_of_content();
}

std::string stringify(unsigned long x) {
	ostringstream ss;
	ss << x;
	return ss.str();
}


TEST_CASE("Strict Parser") {

	SECTION("Reading Words") {
		REQUIRE( test_word("a") == "a" );
		REQUIRE( test_word("  lol ") == "lol" );
		REQUIRE( test_word("\tx86_64\n\ti386\n") == "x86_64");
		REQUIRE_THROWS_AS( test_word("  \n \t\t\r\n \t"), StrictParser::ParseError );
	}

	SECTION("Whitespace treatment") {
		// On a stringstream with skipws (the default behavior):
		REQUIRE( test_integer("42 ", true) == 42 );
		REQUIRE( test_integer(" 42", true) == 42 );

		// On a stringstream with noskipws, the cursor must be on an integer.
		// (so spaces are allowed after, but not before)
		REQUIRE( test_integer("42 ", false) == 42 );
		REQUIRE_THROWS_AS( test_integer(" 42", false), StrictParser::ParseError );
	}

	SECTION("Reading Lines") {
		SECTION("Simple lines") {
			REQUIRE( test_line("i love newlines\n") == "i love newlines" );
			REQUIRE( test_line("i hate newlines")   == "i hate newlines" );
			REQUIRE( test_line("line 1\n line 2")   == "line 1" );
			REQUIRE( test_line("\n") == "" );
			REQUIRE( test_line(" \t ") == " \t " );
			REQUIRE_THROWS_AS( test_line(""), StrictParser::ParseError );
		}
		SECTION("Location of cursor after expect_line") {
			istringstream ss("Line 1\nLine 2");
			StrictParser parser(ss);

			parser.expect_line(); // reading line with \n
			REQUIRE( ss.tellg() == 7 ); // position should be AFTER '\n'
			REQUIRE( (char)ss.peek() == 'L' );

			parser.expect_line(); // reading to end of stream
			REQUIRE( ss.eof() );

			// No more lines
			REQUIRE_THROWS_AS( parser.expect_line(), StrictParser::ParseError );
		}
	}

	SECTION("Reading Integers") {
		
		SECTION("Simple integers") {
			REQUIRE( test_integer("7") == 7 );
			REQUIRE( test_integer("11 12") == 11 );
			REQUIRE( test_integer("-42") == -42 );
			REQUIRE( test_integer("+20") == 20 );
			REQUIRE_THROWS_AS( test_integer(""), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_integer("a1"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_integer("1a"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_integer("1.3"), StrictParser::ParseError );
		}

		SECTION("Integer range limit") {
			// StrictParser is currently constrained to the limits of a long.
			// (this test serves as a reminder to update the documentation if/when this changes!)

			// unsigned long because signed long obviously cannot represent LONG_MAX + 1
			unsigned long big = LONG_MAX;
			unsigned long bigger = big + 1ull;
			
			REQUIRE( test_integer(stringify(big).c_str()) == LONG_MAX );
			REQUIRE_THROWS_AS( test_integer(stringify(bigger).c_str()), StrictParser::ParseError );
		}
	}

	SECTION("Reading Floats") {

		SECTION("Simple floats") {
			REQUIRE( test_float("1 2 3") == Approx(1.0) );
			REQUIRE( test_float("-1.2") == Approx(-1.2) );
			REQUIRE( test_float("3.") == Approx(3.) );
			REQUIRE( test_float("+420E-1") == Approx(+420E-1) );
			REQUIRE( test_float("1.1e2") == Approx(1.1e2) );
			REQUIRE_THROWS_AS( test_float(""), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_float("a1"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_float("1.3a"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_float("inf"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_float("nan"), StrictParser::ParseError );
		}

		SECTION("Float magnitude limits") {
			// Doubles of extremely large magnitude do not parse
			REQUIRE_THROWS_AS( test_float("1e100000000"), StrictParser::ParseError );

			// Extremely small magnitude just goes to zero
			REQUIRE( test_float("1e-100000000") == Approx(0.) );

			// This class makes no promises about subnormal numbers;  don't bother.
		}
	}

	SECTION("Asserting End of Content") {
		SECTION("Simple failure cases") {
			REQUIRE_THROWS_AS( test_end_of_content("a"), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_end_of_content("  lol "), StrictParser::ParseError );
			REQUIRE_THROWS_AS( test_end_of_content("\n\tx86_64\n\ti386\n"), StrictParser::ParseError );
		}
		SECTION("Successful case") {
			string s = "  \n \t\t\r\n \t"; // all whitespace
			stringstream ss(s);
			StrictParser parser(ss);

			REQUIRE( !ss.eof() );
			REQUIRE_NOTHROW( parser.expect_end_of_content() );
			REQUIRE( ss.eof() ); // promised by documentation
			REQUIRE_NOTHROW( parser.expect_end_of_content() ); // multiple calls okay
		}
	}
}
