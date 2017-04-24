#include <vector>
#include <string>
#include <iostream>
#include <math.h>
#include "field2.h"
#include "monitors/monitor.h"
#include "monitors/box_monitor.h"

using namespace std;

namespace qbox {
    box_monitor::box_monitor(string name, const volume &vol, const freq_data &freq_in, bool extendable):
                    monitor(name, "box_monitor", freq_in, extendable), vol(vol) {

        monitors[0] = surface_monitor(name + "_1", vol.get_surface(direction::y_bottom), freq, extendable);
        monitors[1] = surface_monitor(name + "_2", vol.get_surface(direction::x_top),    freq, extendable);
        monitors[2] = surface_monitor(name + "_3", vol.get_surface(direction::y_top),    freq, extendable);
        monitors[3] = surface_monitor(name + "_4", vol.get_surface(direction::x_bottom), freq, extendable);
    }

    box_monitor::box_monitor(const volume &vol, const freq_data &freq_in, bool extendable):
                    monitor("box_monitor", freq_in, extendable), vol(vol) {

        monitors[0] = surface_monitor(name + "_1", vol.get_surface(direction::y_bottom), freq, extendable);
        monitors[1] = surface_monitor(name + "_2", vol.get_surface(direction::x_top),    freq, extendable);
        monitors[2] = surface_monitor(name + "_3", vol.get_surface(direction::y_top),    freq, extendable);
        monitors[3] = surface_monitor(name + "_4", vol.get_surface(direction::x_bottom), freq, extendable);
    }

    void box_monitor::set_F(Field2D *newF) {
        monitor::set_F(newF);
        for (int i = 0; i != 4; i++) 
            monitors[i].set_F(newF);

        auto group = get_group();
        vol.write(group);
    }

    void box_monitor::update() {
        for (int i = 0; i != 4; i++) 
            monitors[i].update();
    }

    Array box_monitor::compute_flux() const {
        double sign_values[] = {-1, -1, 1, 1};
        Array S = Array::Zero(freq.size());

        for (int i = 0; i != 4; i++) {
            Array Sm = sign_values[i]*monitors[i].compute_flux();
            S += Sm;
        }
        return S;
    }

    ComplexArray box_monitor::ntff(const vec &p) const {
        vec center = vol.center();
        double sign_values[] = {1, 1, -1, -1};
        ComplexArray result = ComplexArray::Zero(freq.size());

        for (int i = 0; i != 4; i++)
            result += sign_values[i]*monitors[i].ntff(center, p);

        return result;
    }

    ComplexTensor box_monitor::ntff_sphere(double radius, int N) const {
        ComplexTensor result(N, freq.size());
        Array theta = Array::LinSpaced(N, 0, 2*M_PI);

        for (int i = 0; i < N; i++) {
            vec p = radius*vec(cos(theta[i]), sin(theta[i]));
            ComplexArray p_ntff = ntff(p);
            for (int j = 0; j < freq.size(); j++)
                result(i,j) = p_ntff(j);
        }
        return result;
    }

    void box_monitor::write_ntff_sphere(double radius, int N) const {
        ComplexTensor result = ntff_sphere(radius, N);

        auto my_group = get_group();
        auto dset = write_tensor<complex<double>,2,h5cpp::dtype::Complexd>(my_group, result, "ntff");
        auto attr = dset.create_attribute("radius", h5cpp::dtype::Double);
        attr.write(&radius);
    }

    void box_monitor::write_flux_sides() {
        for (int i = 0; i != 4; i++) 
            monitors[i].write_flux();
    }
}

