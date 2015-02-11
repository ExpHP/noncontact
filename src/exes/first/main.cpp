#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <limits>

#include "../../potential-lj.hpp"

using namespace std;

int main(int argc, char * argv[])
{
	double px = 0.2;
	double py = 0.3;
	double pz = 0.0;
	double length_unit = 0.3;

	double zstart = 0.3;
	double zend   = 0.9;
	size_t zcount = 15;

	LJPotential<double> lj{};
	lj.add_particle(px, py, pz, 1.0, length_unit);

	cout << setprecision(numeric_limits<double>::digits10+1);
	vector<double> v;
	for (int k=0; k<zcount; k++) {
		double z = zstart + (zend-zstart) / (zcount-1) * k;
		cout << z << "\t";
		cout << lj.value_at(px, py, z) << "\t";
		cout << endl;
	}

	return 0;
}
