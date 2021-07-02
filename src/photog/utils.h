#ifndef PHOTOG_PHOTOG_UTILS_H
#define PHOTOG_PHOTOG_UTILS_H

#include <array>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>

#include "doctest/doctest.h"
#include "Halide.h"

#include "constants.h"

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

    photog::Layout get_layout();

    template<typename T>
    Halide::Runtime::Buffer<T>
    get_buffer(float *data, int width, int height, int channels) {
        photog::Layout layout = photog::get_layout();

        if (layout == Layout::Planar)
            return Halide::Runtime::Buffer<T>{data, {width, height, channels}};
        else if (layout == Layout::Interleaved)
            return Halide::Runtime::Buffer<float>::make_interleaved(data, width,
                                                                    height,
                                                                    channels);
        else {
            std::cerr << "Unsupported image layout " << static_cast<int>(layout)
                      << " in photog::get_buffer()." << std::endl;
            abort();
        }
    }
}

TEST_CASE ("testing mul_33_by_31") {
    std::array<float, 9> a_array{1.0, 2.0, 3.0,
                                 4.0, 5.0, 6.0,
                                 7.0, 8.0, 9.0};
    Halide::Runtime::Buffer<float> a{a_array.data(), {3, 3}};
    std::array<float, 3> b_array{2.0, 2.0, 2.0};
    Halide::Runtime::Buffer<float> b{b_array.data(), 3};
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(b);

    output = photog::mul_33_by_31(a, b);

            CHECK(output(0) == doctest::Approx(12.0));
            CHECK(output(1) == doctest::Approx(30.0));
            CHECK(output(2) == doctest::Approx(48.0));
}

TEST_CASE ("testing mul_33_by_33") {
    const int output_dim = 3;
    std::array<float, 9> a_array{1.0, 2.0, 3.0,
                                 4.0, 5.0, 6.0,
                                 7.0, 8.0, 9.0};
    Halide::Runtime::Buffer<float> a{a_array.data(), {output_dim, output_dim}};
    std::array<float, 9> b_array{1.0, 2.0, 3.0,
                                 4.0, 5.0, 6.0,
                                 7.0, 8.0, 9.0};
    Halide::Runtime::Buffer<float> b{b_array.data(), {output_dim, output_dim}};
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(b);

    output = photog::mul_33_by_33(a, b);

    float expected_output[output_dim][output_dim]{{30,  36,  42},
                                                  {66,  81,  96},
                                                  {102, 126, 150}};

    for (int i = 0; i < output_dim; ++i) {
        for (int j = 0; j < output_dim; ++j) {
                    CHECK(
                    output(i, j) == doctest::Approx(expected_output[j][i]));
        }
    }
}

TEST_CASE ("testing div_vec_by_vec") {
    const int output_dim = 3;
    std::array<float, output_dim> a_array{3.0, 4.0, 5.0};
    Halide::Runtime::Buffer<float> a{a_array.data(), output_dim};
    std::array<float, output_dim> b_array{3.0, 2.0, 1.0};
    Halide::Runtime::Buffer<float> b{b_array.data(), output_dim};

    Halide::Runtime::Buffer<float> output = photog::div_vec_by_vec(a, b);

    float expected_output[output_dim]{1.0, 2.0, 5.0};

    for (int i = 0; i < output_dim; ++i) {
                CHECK(output(i) == doctest::Approx(expected_output[i]));
    }
}

TEST_CASE ("testing create_diagonal") {
    const int output_dim = 3;
    std::array<float, output_dim> source_array{1.0, 2.0, 3.0};
    Halide::Runtime::Buffer<float> source{source_array.data(), output_dim};
    Halide::Runtime::Buffer<float> output = photog::create_diagonal(source);

    float expected_output[output_dim][output_dim]{{1, 0, 0},
                                                  {0, 2, 0},
                                                  {0, 0, 3}};

    for (int i = 0; i < output_dim; ++i) {
        for (int j = 0; j < output_dim; ++j) {
                    CHECK(
                    output(i, j) == doctest::Approx(expected_output[j][i]));
        }
    }
}

#endif //PHOTOG_PHOTOG_UTILS_H
