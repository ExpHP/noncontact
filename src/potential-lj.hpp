#pragma once

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

		std::size_t size() const { return this->particles.size(); }

		T value_at(T x, T y, T z) const
		{
			T result = 0.0;
			for (auto particle : this->particles) {
				T delta_x = x - particle.x;
				T delta_y = y - particle.y;
				T delta_z = z - particle.z;

				T delta_r2 = delta_x*delta_x + delta_y*delta_y + delta_z*delta_z;
				T r_minus2 = (particle.length_unit * particle.length_unit) / delta_r2;

				T r_minus6  = r_minus2 * r_minus2 * r_minus2;

				result += particle.energy_unit * (r_minus6 - 2.0) * r_minus6;
			}
			return result;
		}

	private:
		std::vector<LJParticle<T>> particles;
};
