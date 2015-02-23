#include <stdexcept>
#include <utility>
#include <sstream>
#include <istream>
#include <cassert>

// StrictParser has methods for reading data from an input stream according to a
//  known format.  Most reading is done through `operator>>` on the input stream
//  (thus whitespace will be generally be treated according to the stream's `skipws`
//  flag).  However, stricter requirements are placed on the input; whereas operator>>
//  would typically read "23foo" as 23, StrictParser will instead raise an exception.
//
// StrictParser is intended to be used for making runtime assertions on the format
//  of a data file.  It is NOT intended to be used for control flow (like parsing a
//  file differently based on whether a given token in it is an integer or a string).
// To this end, the wrapped input stream is generally left in an unspecified state
//  upon failure; StrictParser does not necessarily make any attempt to put back any
//  characters that it read before throwing a ParseError.
class StrictParser
{
public:

	StrictParser(std::istream & stream)
	: _in(stream)
	{ }

	struct ParseError : public std::runtime_error {
		template <class... Args>
		ParseError(Args&&... args)
		: std::runtime_error(std::forward<Args>(args)...)
		{ }
	};

	// Returns the next contiguous string of non-whitespace characters from the stream.
	// Throws a StrictParser::ParseError if the stream only contains whitespace.
	std::string expect_word () {
		std::string word;
		if (!(_in >> word))
			throw ParseError("Expected word, found end.");
		return word;
	}

	// Reads a line of text from the input stream, advancing the wrapped stream.
	// This is done according to the specifications of std::getline (notably, the
	//  newline separator, if present, will be consumed from the stream, but
	//  excluded from the output).
	// Throws a StrictParser::ParseError if there are no lines left.
	std::string expect_line () {
		std::string line;
		if (!std::getline(_in, line)) {
			if (_in.eof())
				throw ParseError("Unexpected end of stream while reading line.");
			else
				throw ParseError("Unknown error while reading line.");
		}
		return line;
	}

	// If the next word in the stream is a base 10 integer (with nothing extra),
	//  return it and advance the stream.
	// Throws a StrictParser::ParseError if no words can be read, if the complete word
	//  is not a valid integer, or if the integer cannot fit in a long.
	// (examples of success: "1", "-42", "+2", a string encoding LONG_MAX)
	// (examples of failure: "a1", "1a", "0x32", "1.3", a string encoding (LONG_MAX+1))
	long expect_integer () {
		std::string word;
		if (!(_in >> word))
			throw ParseError("Expected integer, found end.");

		long result;
		std::stringstream ss(word);
		ss >> result;

		if (ss.fail() || !ss.eof()) {
			std::string errmsg;
			errmsg = errmsg + "Expected integer, found \"" + word + "\"";
			throw ParseError(errmsg);
		}

		return result;
	}

	// If the next word in the stream is a base 10 floating point number (with nothing extra),
	//  return it and advance the stream.
	// Throws a StrictParser::ParseError if no words can be read, if the complete word is
	//  not a valid floating point number, or if it is too large to be represented with a double.
	// It does not recognize special names like "inf".
	// (examples of success: "1", "-1.2", "+420E-1", "1.1e2", "1e-100000000" (parsed as 0))
	// (examples of failure: "a1", "1.3a", "inf", "nan", "1e10000000")
	double expect_float () {
		std::string word;
		if (!(_in >> word))
			throw ParseError("Expected float, found end.");

		double result;
		std::stringstream ss(word);
		ss >> result;

		if (ss.fail() || !ss.eof()) {
			std::string errmsg;
			errmsg = errmsg + "Expected float, found \"" + word + "\"";
			throw ParseError(errmsg);
		}

		return result;
	}

	// Reads any empty space remaining on the line, and advances the wrapped stream
	//  to the beginning of the next line (or to the end of the file), similarly to
	//  expect_line().
	// Unlike expect_line(), expect_end_of_line() expects the rest of the line to
	//  be empty. A StrictParser::ParseError is thrown if any tokens are encountered.
	void expect_end_of_line () {
		auto line = expect_line();

		std::istringstream stream(line);
		std::string word;

		if (stream >> word) {
			std::string errmsg;
			errmsg = errmsg + "Expected end of line, found \"" + word + "\"";
			throw ParseError(errmsg);
		}
	}

	// If there are no non-whitespace characters left in the stream, advances the input
	//  stream to the end.
	// Otherwise, leaves the stream in an undefined state and throws a StrictParser::ParseError.
	void expect_end_of_content () {
		std::string word;
		if (_in >> word) {
			std::string errmsg;
			errmsg = errmsg + "Expected end of stream, found \"" + word + "\"";
			throw ParseError(errmsg);
		}
		assert(_in.eof());
	}

	// TODO: The above methods are pretty limited, and the class could probably benefit from some
	//  more general methods (like expecting a literal string, whitespace, or a regex), but those
	//  can wait until I really need them.

private:
	std::istream & _in;
};
