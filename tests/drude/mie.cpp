#include <qbox.h>
#include <iostream>

using namespace std;
using namespace qbox;

int main() {
    int pml_thickness = 8;
    double res = 2;
    double f = 2/30.0;
    auto dft = freq_data(1/30.0,3/30.0, 200);

    grid_properties grid(120,120,res,pml_thickness);
    Field2D scat(grid, "scat.h5");

    scat.set_tfsf(volume({60,60}, 65), gaussian_time(f, 1/100.0, 80));
    scat.set_tfsf_freq(dft);

    object o1(cylinder(20), vec(60,60), vec(1,1));
    //scat.add_object(o1, drude(2,5/30.0,0.1));
    Array omega_0(2), gamma(2);
    omega_0 << 5/30.0, 7/30.0;
    gamma << 0.1,0.1;

    scat.add_object(o1, drude(2,omega_0, gamma));

    cylinder_monitor box_scat("box_scat", cylinder_surface(vec(60,60), 50), dft); 
    scat.add_monitor(box_scat);


    for (int i = 0; i != 8000; i++) {
        scat.update();
        //scat.writeE();
    }
    box_scat.write_flux();
    scat.write_tfsf();
}
