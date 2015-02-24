#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <cassert>

#include "../lattice.hpp"

// Forward declarations
class VaspLatticeCell;
class VaspParticleList;

struct VaspLocpot;



class VaspLatticeCell {
public:

	VaspLatticeCell ()
	: _scaleMode(FIXED_SCALE)
	, _scale(1.)
	, _volume(1.)
	, _vectors {{1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.}}
	{ }

	typedef std::array<double, 3> Point;

	// FIXME not implementing until I have proper unit tests
	// coordinate conversion functions
	Point from_direct (Point coords) const;
	Point from_scaled (Point coords) const;
	Point to_direct (Point coords) const;
	Point to_scaled (Point coords) const;

	void set_scale (double scale) {
		_scaleMode = FIXED_SCALE;
		_scale = scale;
		update_scale_volume_invariant();
	}

	void set_volume (double volume) {
		_scaleMode = FIXED_VOLUME;
		_volume = volume;
		update_scale_volume_invariant();
	}

	void set_vector (std::size_t i, Point vector) {
		std::copy(std::begin(vector), std::end(vector), std::begin(_vectors[i]));
		update_scale_volume_invariant();
	}

	// point of clarification: returns unscaled vector
	inline Point vector (std::size_t i) const {
		return {{ _vectors[i][0], _vectors[i][1], _vectors[i][2] }};
	}

	inline double scale ()  const { return _scale; }
	inline double volume () const { return _volume; }

private:
	void update_scale_volume_invariant () {
		double determinant = compute_matrix_determinant();

		switch (_scaleMode) {
			case FIXED_SCALE:
				_volume = pow(_scale,3) * determinant;
				break;
			case FIXED_VOLUME:
				_scale  = pow(_volume/determinant, 1./3.);
				break;
		}

		// the invariant should be met
		assert( fabs((pow(_scale,3) * determinant - _volume)/_volume) < 1E-9 );
	}

	double compute_matrix_determinant () const {
		// elements of first vector times sub-determinants
		double total = 0;
		for (std::size_t i=0; i<3; ++i) {
			std::size_t j = (i+1)%3;
			std::size_t k = (i+2)%3;

			double subdeterminant = 0;
			subdeterminant += _vectors[1][j]*_vectors[2][k];
			subdeterminant -= _vectors[1][k]*_vectors[2][j];

			total += _vectors[0][i] * subdeterminant;
		}
		return total;
	};

	enum {
		FIXED_SCALE,
		FIXED_VOLUME,
	} _scaleMode;
	double _scale;
	double _volume; // INVARIANT: _volume = pow(_scale,3) * matrix_determinant()
	double _vectors[3][3];
};


class VaspParticleList {
public:

	// TODO: the coordinates and dynamics flags are currently not needed, and due to
	//       API concerns (what with the distinction between direct/cartesian coords,
	//       and especially with the fact that the dynamics flags are not meaningful
	//       in full dynamics mode), they're not yet fully implemented.

	static constexpr std::array<double,3> DEFAULT_COORDS {{0.,   0.,   0.}};

	VaspParticleList ()
	: _speciesEndpoints(1, 0.) // first element of this must always be 0
	, _coords()
	{ }

	// number of particles
	std::size_t size () const {
		return _speciesEndpoints.back();
	}

	std::size_t num_species () const {
		assert(_speciesEndpoints.size() >= 1); // check invariant (else we could underflow)
		return _speciesEndpoints.size() - 1;
	}

	// Adds a new species with the specified number of particles to the list,
	//  initializing their positions and dynamics to the default.
	// Returns the index of the first particle added.
	std::size_t add_species (std::size_t count) {
		std::size_t oldsize = size();
		std::size_t newsize = oldsize + count;

		_speciesEndpoints.push_back(newsize);

		_coords.resize(newsize, DEFAULT_COORDS);

		assert(size() == newsize);
		return oldsize; // index of first new particle
	}

	// Returns species index of particle
	// (species are numbered starting from 0, in the order they are added)
	std::size_t species (std::size_t index) const {
		// linear search (# species is expected to be very small)

		for (std::size_t species=0; species < num_species(); ++species) {
			// is index before the beginning of the next species?
			if (index < _speciesEndpoints[species+1])
				return species;
		}

		// If we haven't returned yet, the index must have been too large
		assert(index >= size());
		throw std::out_of_range ("");
	}

	// FIXME: This method name is deliberately ambiguous with regards to coordinate system
	//        (direct/cartesian) because I haven't really decided yet what it should hold.
	std::array<double, 3> & coords (std::size_t index) { return _coords[index]; };
	const std::array<double, 3> coords (std::size_t index) const { return _coords[index]; };

private:

	std::vector<std::size_t> _speciesEndpoints;
	std::vector<std::array<double,3>> _coords;

	// invariant: _coords.size() == size()
	// invariant: _speciesEndpoints.size() >= 1
	// invariant: _speciesEndpoints[0] == 0
	// invariant: _speciesEndpoints.back() == size()
};

// Despite its complete declaration __and__ initialization above, this is required for some reason.
// dangit c++ y u no sanity
constexpr std::array<double,3> VaspParticleList::DEFAULT_COORDS;

// Represents a complete LOCPOT file
// (this is an aggregate type, so it may be initialized using brace syntax)
struct VaspLocpot {
	std::string      title;
	VaspLatticeCell  latticeCell;
	VaspParticleList particleList;
	Lattice3<double> potential;
};
