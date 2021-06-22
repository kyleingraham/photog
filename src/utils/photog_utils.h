#ifndef PHOTOG_PHOTOG_UTILS_H
#define PHOTOG_PHOTOG_UTILS_H

#include <type_traits>

#include "Halide.h"

namespace photog {
    template<typename T>
    Halide::Runtime::Buffer<T>
    mul_33_by_31(const Halide::Runtime::Buffer<T> &a,
                 const Halide::Runtime::Buffer<T> &b) {
        const int output_dim = 3;
        assert(a.dimensions() == 2 && b.dimensions() == 1);
        assert(a.dim(0).extent() == output_dim &&
               b.dim(0).extent() == output_dim);
        assert(a.dim(1).extent() == output_dim);
        Halide::Runtime::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i) {
            output(i) =
                    a(0, i) * b(0) +
                    a(1, i) * b(1) +
                    a(2, i) * b(2);
        }

        return output;
    }

    template<typename T>
    Halide::Runtime::Buffer<T>
    mul_33_by_33(const Halide::Runtime::Buffer<T> &a,
                 const Halide::Runtime::Buffer<T> &b) {
        const int output_dim = 3;
        assert(a.dimensions() == 2 && b.dimensions() == 2);
        assert(a.dim(0).extent() == output_dim &&
               b.dim(0).extent() == output_dim);
        assert(a.dim(1).extent() == output_dim &&
               b.dim(1).extent() == output_dim);
        Halide::Runtime::Buffer<T> output(output_dim, output_dim);
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
    Halide::Runtime::Buffer<T>
    div_vec_by_vec(const Halide::Runtime::Buffer<T> &a,
                   const Halide::Runtime::Buffer<T> &b) {
        assert(a.dimensions() == 1 && b.dimensions() == 1);
        assert(a.dim(0).extent() == b.dim(0).extent());
        int output_dim = a.dim(0).extent();
        Halide::Runtime::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i)
            output(i) = a(i) / b(i);

        return output;
    }

    template<typename T>
    Halide::Runtime::Buffer<T>
    create_diagonal(const Halide::Runtime::Buffer<T> &source) {
        assert(source.dimensions() == 1);
        int output_dim = source.dim(0).extent();
        Halide::Runtime::Buffer<T> output(output_dim, output_dim);
        for (int i = 0; i < output_dim; ++i) {
            for (int j = 0; j < output_dim; ++j) {
                if (i == j)
                    output(i, j) = source(i);
                else
                    output(i, j) = 0.0f;
            }
        }

        return output;
    }

    template<typename T, size_t N>
    Halide::Runtime::Buffer<T>
    copy_to_buffer(std::array<T, N> values, int square_size) {
        assert((square_size * square_size) == N);
        auto output = Halide::Runtime::Buffer<T>(square_size, square_size);
        int base = 0;
        for (int y = 0; y < square_size; ++y) {
            for (int x = 0; x < square_size; ++x) {
                // x, y => i
                // 0, 0 => 0 <- base = 0
                // 1, 0 => 1
                // 2, 0 => 2
                // 0, 1 => 3 <- base = 2
                // 1, 1 => 4
                // 2, 1 => 5
                // 0, 2 => 6 <- base = 4
                // 1, 2 => 7
                // 2, 2 => 8
                output(x, y) = values[base + x + y];
            }
            base = base + (square_size - 1);
        }

        return output;
    }

    template<typename T, size_t N>
    Halide::Runtime::Buffer<T>
    copy_to_buffer(std::array<T, N> values) {
        auto output = Halide::Runtime::Buffer<T>(N);
        for (int x = 0; x < N; ++x)
            output(x) = values[x];

        return output;
    }

    template<typename Array, size_t N, typename T = typename std::remove_cv<Array>::type>
    Halide::Runtime::Buffer<T>
    copy_to_buffer(Array (&values)[N]) {
        // Works for: T array[N]{...};
        static_assert(!(std::is_void<T>::value),
                      "Function does not support void arrays.");
        auto output = Halide::Runtime::Buffer<T>(N);
        for (int x = 0; x < N; ++x)
            output(x) = values[x];

        return output;
    }
}

#endif //PHOTOG_PHOTOG_UTILS_H
