#ifndef PHOTOG_PHOTOG_UTILS_H
#define PHOTOG_PHOTOG_UTILS_H

#include "Halide.h"

namespace photog {
    template<typename T>
    Halide::Buffer<T>
    mul_33_by_31(const Halide::Buffer<T> &a, const Halide::Buffer<T> &b) {
        static int output_dim = 3;
        Halide::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i) {
            output(i) =
                    a(0, i) * b(0) +
                    a(1, i) * b(1) +
                    a(2, i) * b(2);
        }

        return output;
    }

    template<typename T>
    Halide::Buffer<T>
    mul_33_by_33(const Halide::Buffer<T> &a, const Halide::Buffer<T> &b) {
        static int output_dim = 3;
        Halide::Buffer<T> output(output_dim, output_dim);
        for (int i = 0; i < output_dim; ++i) {
            output(0, i) = a(0, i) * b(0, 0) +
                           a(1, i) * b(0, 1) +
                           a(2, i) * b(0, 2);
            output(1, i) = a(0, i) * b(1, 0) +
                           a(1, i) * b(1, 1) +
                           a(2, i) * b(1, 2);
            output(2, i) = a(0, i) * b(2, 0) +
                           a(1, i) * b(2, 1) +
                           a(2, i) * b(2, 2);
        }

        return output;
    }

    template<typename T>
    Halide::Buffer<T>
    div_31_by_31(const Halide::Buffer<T> &a, const Halide::Buffer<T> &b) {
        assert(a.dimensions() == 1 && b.dimensions() == 1);
        assert(a.dim(0).extent() == 3 && b.dim(0).extent() == 3);
        static int output_dim = 3;
        Halide::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i) {
            output(i) = a(i) / b(i);
        }

        return output;
    }

    template<typename T>
    Halide::Buffer<T>
    create_diagonal(const Halide::Buffer<T> &source) {
        assert(source.dimensions() == 1);
        int output_dim = source.dim(0).extent();
        Halide::Buffer<T> output({output_dim, output_dim});
        for (int i = 0; i < output_dim; ++i) {
            output(i, i) = source(i);
        }

        return output;
    }
}

#endif //PHOTOG_PHOTOG_UTILS_H