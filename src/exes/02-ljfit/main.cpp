#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <limits>
#include <cmath>

#include "../../potential-lj.hpp"
#include "../../pseudopotential-lj.hpp"
#include "../../lattice.hpp"

#ifdef PLOT_POT
const char DESCRIPTION[] = "Outputs matrix of LJ potential data.";
#endif

#ifdef PLOT_FIT
const char DESCRIPTION[] = "Plots fitted LJ potential.";
#endif

using namespace std;

int main(int argc, char * argv[])
{
	if (argc != 6) {
		cerr << argv[0] << ": " << DESCRIPTION << endl;
		cerr << "Usage: " << argv[0] << "  N_SAMPLE  N_IMAGE  FIT_LB  FIT_UB  ZPOS" << endl;
		return 1;
	}

	size_t arg_nsample = atoi(argv[1]);
	size_t arg_nimage  = atoi(argv[2]);
	double arg_lb   = atof(argv[3]);
	double arg_ub   = atof(argv[4]);
	double arg_zpos = atof(argv[5]);

	double length_unit = 0.3;

	LJPotential<double> lj {};
	lj.add_particle(0.25, 0.25, 0.0, 1.0, length_unit);
	lj.add_particle(0.25, 0.75, 0.0, 1.0, length_unit);
	lj.add_particle(0.75, 0.25, 0.0, 1.0, length_unit);
	lj.add_particle(0.75, 0.75, 0.0, 1.0, length_unit);
	lj.add_particle(0.5,  0.5,  0.0, 0.5, length_unit);

	cout << setprecision(numeric_limits<double>::digits10+1);

	auto data = Lattice3<double> {arg_nsample, arg_nsample, 15}
		.set_lower_coords(0., 0., arg_lb)
		.set_upper_coords(1., 1., arg_ub)
	;

	for (std::size_t i=0; i < data.axis_size(1); i++) {
		for (std::size_t j=0; j < data.axis_size(2); j++) {
			for (std::size_t k=0; k < data.axis_size(3); k++) {
				double x = data.coord(1,i);
				double y = data.coord(2,j);
				double z = data.coord(3,k);
				data(i, j, k) = lj.value_at(x, y, z);
			}
		}
	}

	// fit the data
	LJPseudoPotential<double> fit = LJPseudoPotential<double>::fit_to_data(data);

	size_t imgdim = arg_nimage;
	for (size_t i=0; i < imgdim; i++) {
		for (size_t j=0; j < imgdim; j++) {
			double x = 1. / (imgdim-1) * i;
			double y = 1. / (imgdim-1) * j;
			double z = arg_zpos;

			#ifdef PLOT_FIT
			cout << fit.value_at(x, y, z) << "\t";
			#endif

			#ifdef PLOT_POT
			cout << lj.value_at(x, y, z) << "\t";
			#endif
		}
		cout << endl;
	}
	return 0;
}
