#pragma once

#include "external-deps/catch.hpp"

#include <vector>

template <class T>
struct LJParticle {
	T x; // x position
	T y; // y position
	T z; // z position
	T energy_unit; // $\epsilon$ on Wikipedia.  Absolute value of minimum energy (which is negative).
	T length_unit; // $r_m$ on Wikipedia.  Distance at which minimum energy occurs.

	// Constructor
	LJParticle(T x, T y, T z, T energy_unit, T length_unit)
	: x(x), y(y), z(z)
	, energy_unit(energy_unit)
	, length_unit(length_unit)
	{ }
};

template <class T>
class LJPotential {
	public:
		void add_particle(T x, T y, T z, T energy_unit, T length_unit)
		{
			particles.emplace_back(x, y, z, energy_unit, length_unit);
		}

		size_t size() const { return this->particles.size(); }

		T value_at(T x, T y, T z) const
		{
			T result = 0.0;
			for (auto particle : this->particles) {
				T delta_x = x - particle.x;
				T delta_y = y - particle.y;
				T delta_z = z - particle.z;

				T delta_r2 = delta_x*delta_x + delta_y*delta_y + delta_z*delta_z;
				T unitless_r2 = delta_r2 / (particle.length_unit * particle.length_unit);

				T unitless_r6  = unitless_r2 * unitless_r2 * unitless_r2;

				result += particle.energy_unit * (unitless_r6 - 2.0) * unitless_r6;
			}

			//std::cout << "value_at(" << x << ", " << y << ", " << z << ") = " << result << std::endl;
			assert( !std::isnan(result) );
			return result;
		}

	private:
		std::vector<LJParticle<T>> particles;
};

TEST_CASE("Parameters of LJ potential correspond to minimum") {
	double emin = 32.;
	double rmin = 5.;

	LJPotential<double> p;
	p.add_particle(10., 10., 10., emin, rmin);

	// one point on the minimum would be at (13,14,10)
	SECTION("-1 * energy_unit occurs at minimum") {
		REQUIRE(p.value_at(13., 14., 10.) == Approx(-emin));
	}

	SECTION("points around minimum have greater potential") {
		REQUIRE(p.value_at(12., 14., 10.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(14., 14., 10.) > p.value_at(13., 14., 10.));

		REQUIRE(p.value_at(13., 13., 10.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(13., 15., 10.) > p.value_at(13., 14., 10.));

		REQUIRE(p.value_at(13., 14.,  9.) > p.value_at(13., 14., 10.));
		REQUIRE(p.value_at(13., 14., 11.) > p.value_at(13., 14., 10.));
	}
}
