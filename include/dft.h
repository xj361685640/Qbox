#ifndef GUARD_freq_h
#define GUARD_freq_h

#include "h5cpp.h"
#include "vec.h"
#include <array>
#include <vector>
#include <complex>
#include <initializer_list>
#include <functional>

namespace qbox {

    using namespace std::complex_literals;

    template<unsigned int RANK>
    struct complex_dft_tensor {
        using dft_tensor = Eigen::Tensor<double, RANK+1, Eigen::RowMajor>;
        complex_dft_tensor(const dft_tensor &real, const dft_tensor &imag): real(real), imag(imag) {};
        dft_tensor real, imag;

        void write(const h5cpp::h5group& group, const std::string &name) const;
            // h5cpp::write_tensor(real, group, 'r' + name);
            // h5cpp::write_tensor(imag, group, 'i' + name);
    };

    //template specialization needed because Eigen can't cast when RANK is a template parameter for reasons I know not
    template<>
    void complex_dft_tensor<0>::write(const h5cpp::h5group& group, const std::string &name) const {
        Eigen::Tensor<std::complex<double>, 1, Eigen::RowMajor> combined = 
                real.cast<std::complex<double>>() + 1i*imag.cast<std::complex<double>>();
        h5cpp::write_tensor(combined, group, name);
    }

    template<>
    void complex_dft_tensor<1>::write(const h5cpp::h5group& group, const std::string &name) const {
        Eigen::Tensor<std::complex<double>, 2, Eigen::RowMajor> combined = 
                real.cast<std::complex<double>>() + 1i*imag.cast<std::complex<double>>();
        h5cpp::write_tensor(combined, group, name);
    }

    template<>
    void complex_dft_tensor<2>::write(const h5cpp::h5group& group, const std::string &name) const {
        Eigen::Tensor<std::complex<double>, 3, Eigen::RowMajor> combined = 
                real.cast<std::complex<double>>() + 1i*imag.cast<std::complex<double>>();
        h5cpp::write_tensor(combined, group, name);
    }

    namespace DFT {
        class Ez{};
        class Hx{};
        class Hy{};
        class tangent{};
        class all{};
    }

    template<unsigned int RANK>
    class dft {
        using dft_tensor  = Eigen::Tensor<double, RANK+1, Eigen::RowMajor>;
        using dft_dimensions = typename dft_tensor::Dimensions;

    public:
        dft() = default;

        dft(const Array &freq): t(0), freq(freq) {};

        dft(const Array &freq, std::string &name, const std::array<int,RANK> &dim): dft(freq) {
            add(name, dim);
        }

        dft(const Array &freq, const std::vector<std::string> &names, const std::array<int,RANK> &dim): dft(freq) {
            add(names, dim);
        }

        void add(const std::string &name, const std::array<int,RANK> &dim) {
            // assert that name doesn't exist already
            // assert that len(dim) = RANK

            dft_dimensions dft_dims;
            for (int i = 0; i < RANK; i++)
                dft_dims[i] = dim[i];
            dft_dims[RANK] = Nfreq();

            fourier.insert({name, complex_dft_tensor<RANK>(dft_tensor(dft_dims), dft_tensor(dft_dims))});
            fourier.at(name).real.setZero();
            fourier.at(name).imag.setZero();
        }

        void add(const std::vector<std::string> &names, const std::array<int,RANK> &dim) {
            for (const auto&& name : names)
                add(name, dim);
        }

        template <class T>
        void update(std::function<std::function<T>(std::string)> f, double tnew);


        void write_properties(const h5cpp::h5group &group) const {
            h5cpp::write_array<double>(freq, group, "frequency");
        }

        void write(const h5cpp::h5group &group) const {
            //write fourier
            for (const auto& p: fourier)
                p.second.write(group, p.first);
        }

        //getter functions
        Array get_freq() const {return freq;}
        double get_freq(int i) const {return freq[i];}
        int Nfreq() const {return freq.size();}

        const complex_dft_tensor<RANK>& operator() (const std::string &name) const {return fourier.at(name);}
        const complex_dft_tensor<RANK>& get(const std::string &name) const {return fourier.at(name);}

        void operator-=(const dft& other) {
            // subtract fourier's only if the following are equal: RANK, freq, fourier's keys
        }

    private:
        double t;     ///< current time
        Array freq;   ///< frequency data

        std::map<std::string, complex_dft_tensor<RANK>> fourier;
    };

    template<>
    template<> void dft<0>::update(std::function<std::function<double()>(std::string)> f, double tnew) {
        if (t == tnew)
            return;
        t = tnew;
        // assert that keys match exactly

        Array cosf = Eigen::cos(2*M_PI*freq*t);
        Array sinf = Eigen::sin(2*M_PI*freq*t);

        const int kmax = Nfreq();
        for (auto& p: fourier) {
            auto g = f(p.first);
            double g_val = g();
            for (int k = 0; k < kmax; k++) {
                p.second.real(k) += g_val*cosf(k);
                p.second.imag(k) += g_val*sinf(k);
            }
        }
    }


    template<>
    template<> void dft<1>::update(std::function<std::function<double(int)>(std::string)> f, double tnew) {
        if (t == tnew)
            return;
        t = tnew;
        // assert that keys match exactly

        Array cosf = Eigen::cos(2*M_PI*freq*t);
        Array sinf = Eigen::sin(2*M_PI*freq*t);

        const int kmax = Nfreq();
        for (auto& p: fourier) {
            auto g = f(p.first);
            const int imax = p.second.real.dimensions()[0];

            for (int i = 0; i < imax; i++ ) {
                double g_val = g(i);
                for (int k = 0; k < kmax; k++) {
                    p.second.real(i,k) += g_val*cosf(k);
                    p.second.imag(i,k) += g_val*sinf(k);
                }
            }
        }
    }
}

#endif