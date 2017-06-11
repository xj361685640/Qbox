#ifndef GUARD_vec_h
#define GUARD_vec_h

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>
#include <eigen3/unsupported/Eigen/CXX11/Tensor>
#include <stdexcept>
#include "h5cpp.h"
#include <vector>
#include <string>
#include <complex>
#include "math.h"

namespace qbox {

    using vec = Eigen::Vector2d;
    using cvec = Eigen::Vector2cd;
    using ivec = Eigen::Vector2i;
    using quat = Eigen::Quaterniond;
    using Array = Eigen::ArrayXd;
    using ComplexArray = Eigen::ArrayXcd;
    using tensor = Eigen::Tensor<double,2,Eigen::RowMajor>;
    using tensor3 = Eigen::Tensor<double,3,Eigen::RowMajor>;
    using tensor4 = Eigen::Tensor<double,4,Eigen::RowMajor>;
    using ComplexTensor = Eigen::Tensor<std::complex<double>,2,Eigen::RowMajor>;
    using ComplexTensor3 = Eigen::Tensor<std::complex<double>,3,Eigen::RowMajor>;
    using ComplexTensor4 = Eigen::Tensor<std::complex<double>,4,Eigen::RowMajor>;

    enum class direction {
        x_bottom, x_top,
        y_bottom, y_top,
        //z_bottom, z_top,
    };

    enum class fields {
        Ez,
        Hx,
        Hy
    };

    struct cylinder_surface {
        cylinder_surface() = default;
        cylinder_surface(vec center, double radius): center(center), radius(radius) {}

        vec tangent(double theta) {
            return vec(-sin(theta), cos(theta));
        }
        vec normal(double theta) {
            return vec(cos(theta), sin(theta));
        }
        vec position(double theta) {
            return radius*normal(theta) + center;
        }

        void write(const h5cpp::h5group &group) const {
            h5cpp::write_vector<double>(center, group, "center");
            h5cpp::write_scalar(radius, group, "radius");
        }

    public:
        vec center;                ///< vector coordinates of center
        double radius;             ///< cylinder radius
    };

    template<class T, h5cpp::dtype M>
    struct surface_template {
        surface_template() = default;
        surface_template(Eigen::Matrix<T,2,1> a, Eigen::Matrix<T,2,1> b, int sign = 1): a(a), b(b), sign(sign) {
            dim = (a-b).cwiseAbs();

            if (dim[0] == 0) {normal = sign*Eigen::Matrix<T,2,1>(1,0);}
            else if (dim[1] == 0) {normal = sign*Eigen::Matrix<T,2,1>(0,1);}
            else throw std::invalid_argument("vectors must have one component equal to each other");
            tangent = (b-a)/(b-a).norm();
        }

        //surface_template(Eigen::Matrix<T,2,1> center, Eigen::Matrix<T,2,1> normal_vec, T l, int sign = 1): sign(sign) {
            //if (normal_vec[0] == 0) {normal = sign*Eigen::Matrix<T,2,1>(0,1);}
            //else if (normal_vec[1] == 0) {normal = sign*Eigen::Matrix<T,2,1>(1,0);}
            //else throw std::invalid_argument("invalid normal vector");

            //a = center - l/2*Eigen::Matrix<T,2,1>(center[1], -center[0]);
            //b = center + l/2*Eigen::Matrix<T,2,1>(center[1], -center[0]);
            //dim = (a-b).cwiseAbs();
        //}

        void write(const h5cpp::h5group &group) const {
            h5cpp::write_vector<T>(a, group, "p1");
            h5cpp::write_vector<T>(b, group, "p2");
        }

    public:
        Eigen::Matrix<T,2,1> a,b;       ///< vector coordinates of two corners
        int sign;                       ///< 
        Eigen::Matrix<T,2,1> dim;       ///< vector containing the dimensions
        Eigen::Matrix<T,2,1> normal;    ///< unit vector normal to the surface
        Eigen::Matrix<T,2,1> tangent;   ///< unit vector that points from a to b
    };

    template<class T, h5cpp::dtype M>
    struct volume_template {
        volume_template() = default;
        volume_template(Eigen::Matrix<T,2,1> a, Eigen::Matrix<T,2,1> b): a(a), b(b) {
            dim = (a-b).cwiseAbs();
        }

        volume_template(Eigen::Matrix<T,2,1> center, T lx, T ly): dim(Eigen::Matrix<T,2,1>(lx,ly)) {
            a = center - Eigen::Matrix<T,2,1>(lx,ly)/2;
            b = center + Eigen::Matrix<T,2,1>(lx,ly)/2;
        }

        volume_template(Eigen::Matrix<T,2,1> center, T l): volume_template(center,l,l) {};

        void write(const h5cpp::h5group &group) const {
            h5cpp::write_vector<T>(a, group, "p1");
            h5cpp::write_vector<T>(b, group, "p2");
        }

        Eigen::Matrix<T,2,1> center() const {
            return (a + b)/2;
        }

        surface_template<T,M> get_surface(direction dir) const {
            switch(dir) {
                case direction::y_bottom:
                    return surface_template<T,M>(a, vec(b[0], a[1])); break;
                case direction::y_top:
                    return surface_template<T,M>(b, vec(a[0], b[1])); break;
                case direction::x_bottom:
                    return surface_template<T,M>(vec(a[0], b[1]), a); break;
                case direction::x_top:
                    return surface_template<T,M>(vec(b[0], a[1]), b); break;
                default: throw std::invalid_argument("not a valid direction");

            }
        }

    public:
        Eigen::Matrix<T,2,1> a,b;
        Eigen::Matrix<T,2,1> dim;
    };

    using surface = surface_template<double, h5cpp::dtype::Double>;
    using isurface = surface_template<int, h5cpp::dtype::Int>;
    using volume = volume_template<double, h5cpp::dtype::Double>;
    using ivolume = volume_template<int, h5cpp::dtype::Int>;

}

#endif
