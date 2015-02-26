#pragma once

#include <array>
#include <cmath>

#include <Eigen/Dense>

#include "../points.hpp"

// VectorBasis is a basis of 3 arbitrary cartesian vectors.  The vectors need not
//  be orthonormal, so long as they are linearly independent.
// If we construct a matrix wherein each row is one of the vectors, then conversion
//  from VectorBasis to Cartesian is a matrix-vector multiply, and conversion
//  into VectorBasis is solving a linear system.
struct VectorBasis
{
public:
	double vectors[3][3];
};

// VectorBasis => Cartesian
template<> auto transform (const Point<VectorBasis> & point, Cartesian basis) -> Point<decltype(basis)>
{
	// matrix vector multiply
	double (*m)[3] = point.basis().vectors;
	return {
		m[0][0]*point.first() + m[0][1]*point.second() + m[0][2]*point.third(),
		m[1][0]*point.first() + m[1][1]*point.second() + m[1][2]*point.third(),
		m[2][0]*point.first() + m[2][1]*point.second() + m[2][2]*point.third(),
		basis
	};
}

// Cartesian => VectorBasis
template<> auto transform (const Point<Cartesian> & point, VectorBasis basis) -> Point<decltype(basis)>
{
	// Eigen wrappers for raw pointers
	Eigen::Map<Eigen::Matrix<double,3,3,Eigen::RowMajor>> matrix (&(basis.vectors[0][0]));
	Eigen::Map<const Eigen::Vector3d> product (point.data());

	// get QR decomposition
	auto solver = matrix.fullPivHouseholderQr();

	// use it to solve the linear system
	Eigen::Vector3d solution = solver.solve(product);

	return {
		solution(0), solution(1), solution(2),
		basis
	};
}

// VectorBasis => VectorBasis
// Nontrivial; we'll let the fallback implementation handle this.
//template<> auto transform (const Point<VectorBasis> & point, VectorBasis basis) -> Point<decltype(basis)>

