#pragma once

#include <istream>
#include <sstream>
#include <string>

#include "vasp-structures.hpp"
#include "strict-parser.hpp"
#include "../util/errors.hpp"

class VaspLocpotLatticeParser;
class VaspLocpotParticleListParser;
class VaspLocpotPotentialParser;


bool is_non_negative_integer (const std::string & s)
{
	auto it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}


// Takes a stream containing the four lines of a LOCPOT or similar file
//  that define the lattice geometry, and produces a VaspLatticeCell.
class VaspLocpotLatticeParser
: private StrictParser
{
public:
	VaspLocpotLatticeParser (std::istream & in)
	: StrictParser(in)
	, _in(in)
	{ }

	VaspLatticeCell parse () {
		VaspLatticeCell obj;

		expect_scale_line(obj);
		expect_vector_lines(obj);

		// NOTE: Would be awesome if these parsers could somehow be given streams that
		//       appear to only contain a specific portion of the file, but any
		//       possible implementation I can think of results in a very leaky
		//       abstraction. (with regards to how reads on the restricted stream
		//       would affect the original stream... and possibly with regards to
		//       methods such as seekg()).
		//
		// expect_end_of_content();

		return obj;
	}

private:
	std::istream & _in;

	void expect_scale_line (VaspLatticeCell & obj) {
		// A line containing a single float
		double scaleFromFile = expect_float();

		if (scaleFromFile < 0.) {
			// FIXME not implementing until I have proper unit tests
			throw LazyProgrammerError("Negative lattice scale factors (which represent the unit cell volume) are not implemented");
		} else {
			obj.set_scale(scaleFromFile);
		}

		expect_end_of_line();
	}

	void expect_vector_lines (VaspLatticeCell & obj) {
		// 3 lines with 3 floats each
		for (int i=0; i<3; i++) {

			obj.set_vector(i, {{expect_float(), expect_float(), expect_float()}});

			expect_end_of_line();
		}
	}
};

class VaspLocpotParticleListParser
: private StrictParser
{
public:
	VaspLocpotParticleListParser (std::istream & in)
	: StrictParser(in)
	, _in(in)
	{ }

	VaspParticleList parse () {
		VaspParticleList obj;

		while (!at_flag_line())
			expect_counts_line(obj);

		while (at_flag_line())
			expect_flag_line(obj);

		expect_particle_lines(obj);
	}

private:
	std::istream & _in;

	bool at_flag_line () const {
		// FIXME: My current best understanding of the format so far is that it is
		//        the *lack of indentation* of the "Selective Dynamics", "Direct" or
		//        "Cartesian" lines that signals their presence.
		return (char)_in.peek() != ' ';
	}

	void expect_counts_line (VaspParticleList & obj) {
		// FIXME: My current best understanding is that any non-integer words that
		//        appear in this section (such as the names of the species) are simply
		//        ignored, as the Vasp documentation provides no indication that they
		//        are expected or even allowed.
		std::string line = expect_line();

		std::stringstream linestream(line);
		std::string word;

		// read each word, ignoring non-integers
		while (linestream >> word) {
			if (is_non_negative_integer(word)) {
				std::size_t count;
				std::stringstream wordstream(word);

				wordstream >> count;

				obj.add_species(count);
			}
		}
	}

	void expect_flag_line (VaspParticleList & obj) {
		std::string line = expect_line();

		char c = line[0]; // first char

		switch (toupper(c)) {
			case ' ':
				// UNREACHABLE
				// (however, it would become reachable if, say, at_flag_line()
				//  was modified without also fixing this method... HINT, HINT)
				throw std::logic_error("the aristocrats!");

			case 'C':
			case 'K':
				set_cartesian_coords();
				break;

			case 'S':
				set_selective_dynamics();
				break;

			default:
				// The Vasp docs place no explicit restriction on the first character of a
				// "Direct coordinates" line (which is a no-op), so we must allow anything.

				// ...nonetheless, I think I'd like to know if it doesn't begin with 'D'.
				if (toupper(c) != 'D') {
					std::cerr << "WARNING: Line beginning with '" << c << "' interpreted as a Direct ";
					std::cerr << "Coordinates marker and ignored." << std::endl;
					std::cerr << "  NOTE: Content was \"" << line << "\"" << std::endl;
				}
		}
	}

	// FIXME
	void set_cartesian_coords() {
		throw LazyProgrammerError("Parsing of cartesian coords not implemented");
	}

	// FIXME
	void set_selective_dynamics() {
		throw LazyProgrammerError("Parsing of selective dynamics not implemented");
	}

	// FIXME
	bool has_selective_dynamics() {
		throw LazyProgrammerError("Parsing of selective dynamics not implemented");
	}

	void expect_particle_lines (VaspParticleList & obj) {
		assert(obj.size() > 0);

		for (std::size_t i = 0; i < obj.size(); ++i) {
			double coord1 = expect_float();
			double coord2 = expect_float();
			double coord3 = expect_float();

			obj.coords(i) = {{coord1, coord2, coord3}};

			if (has_selective_dynamics()) {
				// FIXME The one example they give uses "T" and "F", but I imagine it uses parsing
				//       routines built into FORTRAN, which likely accept a wider variety of inputs.
				throw LazyProgrammerError("Reading selective dynamics flags is not implemented");

				//bool dynamics1 = expect_fortran_logical(); // <-- method name crazy enough to work?
			}

			expect_end_of_line();
		}
	}
};


class VaspLocpotPotentialParser
: private StrictParser
{
public:
	VaspLocpotPotentialParser (std::istream & in)
	: StrictParser(in)
	, _in(in)
	{ }

	Lattice3<double> parse() {
		// The docs claim this can be read format-free

		long dimx = expect_integer();
		long dimy = expect_integer();
		long dimz = expect_integer();

		if ((dimx <= 0) || (dimy <= 0) || (dimz <= 0))
			throw StrictParser::ParseError("Encountered non-positive dimension!");

		Lattice3<double> obj(dimx, dimy, dimz);
		// TODO: Account for lattice cell vectors?

		// numbers are printed in fortran order (x is fast index)
		for (long k=0; k<dimz; ++k)
			for (long j=0; j<dimy; ++j)
				for (long i=0; i<dimx; ++i)
					obj(i,j,k) = expect_float();

		// we're done with format-free reading; scroll to beginning of next line
		// to facilitate line reading
		expect_end_of_line();

		return obj;
	}

private:
	std::istream & _in;
};


class VaspLocpotParser
: private StrictParser
{
public:

	VaspLocpotParser (std::istream & in)
	: StrictParser(in)
	, _in(in)
	{ }

	VaspLocpot parse () {

		auto title        = expect_line();
		auto latticeCell  = VaspLocpotLatticeParser(_in).parse();
		auto particleList = VaspLocpotParticleListParser(_in).parse();
		auto potential    = VaspLocpotPotentialParser(_in).parse();
		expect_end_of_content();

		return VaspLocpot {title, latticeCell, particleList, potential};
	}

private:
	std::istream & _in;
};
