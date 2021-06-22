# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <array>

#include "doctest/doctest.h"
#include "Halide.h"
#include "halide_image_io.h"

#include "photog_color.h"
#include "photog_utils.h"
// Available after a CMake build
#include "photog_srgb_to_linear.h"
#include "photog_rgb_to_linear.h"
#include "photog_srgb_to_xyz.h"
#include "photog_rgb_to_xyz.h"
#include "photog_linear_to_srgb.h"
#include "photog_linear_to_rgb.h"
#include "photog_xyz_to_srgb.h"
#include "photog_xyz_to_rgb.h"
#include "photog_average.h"
#include "photog_chromadapt.h"

TEST_CASE ("testing photog_srgb_to_linear") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_srgb_to_linear(input, output);

    // 0.04045f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(0.423268f));
            CHECK(output(0, 0, 1) == doctest::Approx(0.341914f));
            CHECK(output(0, 0, 2) == doctest::Approx(0.194618f));
    // input(x, y, c) <= 0.04045f
            CHECK(output(1824, 445, 0) == doctest::Approx(0.003035f));
            CHECK(output(1824, 445, 1) == doctest::Approx(0.003035f));
            CHECK(output(1824, 445, 2) == doctest::Approx(0.002428f));
}

TEST_CASE ("testing photog_chromadapt") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    auto output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);
    Halide::Runtime::Buffer<float> source_tristimulus(3);

    photog_average(input, source_tristimulus);
    source_tristimulus =
            photog::rgb_to_xyz(source_tristimulus,
                               photog::get_gamma(PhotogWorkingSpace::Srgb),
                               photog::get_rgb_to_xyz_xfmr(
                                       PhotogWorkingSpace::Srgb));

    Halide::Runtime::Buffer<float> transform =
            photog::create_transform(PhotogChromadaptMethod::Bradford,
                                     source_tristimulus,
                                     PhotogIlluminant::D65);

    photog_chromadapt(input,
                      photog::get_gamma(PhotogWorkingSpace::Srgb),
                      photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb),
                      photog::get_xyz_to_rgb_xfmr(PhotogWorkingSpace::Srgb),
                      transform,
                      output);

    Halide::Tools::convert_and_save_image(output, R"(images/out.jpg)");
}

TEST_CASE ("testing photog_rgb_to_linear") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_rgb_to_linear(input, photog::get_gamma(PhotogWorkingSpace::Srgb),
                         output);

    // 0.04045f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(0.431340f));
            CHECK(output(0, 0, 1) == doctest::Approx(0.348865f));
            CHECK(output(0, 0, 2) == doctest::Approx(0.197516f));
    // input(x, y, c) <= 0.04045f
            CHECK(output(1824, 445, 0) == doctest::Approx(0.000804f));
            CHECK(output(1824, 445, 1) == doctest::Approx(0.000804f));
            CHECK(output(1824, 445, 2) == doctest::Approx(0.000492f));
}

TEST_CASE ("testing photog_srgb_to_xyz") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_srgb_to_xyz(input, output);

    // 0.04045f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(0.331956f));
            CHECK(output(0, 0, 1) == doctest::Approx(0.348585f));
            CHECK(output(0, 0, 2) == doctest::Approx(0.233883f));
    // input(x, y, c) <= 0.04045f
            CHECK(output(1824, 445, 0) == doctest::Approx(0.002775f));
            CHECK(output(1824, 445, 1) == doctest::Approx(0.002991f));
            CHECK(output(1824, 445, 2) == doctest::Approx(0.002728f));
}

TEST_CASE ("testing photog_rgb_to_xyz") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_rgb_to_xyz(input,
                      photog::get_gamma(PhotogWorkingSpace::Srgb),
                      photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb),
                      output);

    // 0.04045f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(0.338294f));
            CHECK(output(0, 0, 1) == doctest::Approx(0.355482f));
            CHECK(output(0, 0, 2) == doctest::Approx(0.237622f));
    // input(x, y, c) <= 0.04045f
            CHECK(output(1824, 445, 0) == doctest::Approx(0.000708f));
            CHECK(output(1824, 445, 1) == doctest::Approx(0.000782f));
            CHECK(output(1824, 445, 2) == doctest::Approx(0.000580f));
}

TEST_CASE ("testing photog_linear_to_srgb") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> linear =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_srgb_to_linear(input, linear);

    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_linear_to_srgb(linear, output);

    // 0.0031308f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(input(0, 0, 0)));
            CHECK(output(0, 0, 1) == doctest::Approx(input(0, 0, 1)));
            CHECK(output(0, 0, 2) == doctest::Approx(input(0, 0, 2)));
    // input(x, y, c) <= 0.0031308f
            CHECK(output(4550, 711, 0) == doctest::Approx(input(4550, 711, 0)));
            CHECK(output(4550, 711, 1) == doctest::Approx(input(4550, 711, 1)));
            CHECK(output(4550, 711, 2) == doctest::Approx(input(4550, 711, 2)));
}

TEST_CASE ("testing photog_linear_to_rgb") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> linear =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_rgb_to_linear(input, photog::get_gamma(PhotogWorkingSpace::Srgb),
                         linear);

    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_linear_to_rgb(linear,
                         photog::get_gamma(PhotogWorkingSpace::Srgb),
                         output);

    // 0.0031308f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(input(0, 0, 0)));
            CHECK(output(0, 0, 1) == doctest::Approx(input(0, 0, 1)));
            CHECK(output(0, 0, 2) == doctest::Approx(input(0, 0, 2)));
    // input(x, y, c) <= 0.0031308f
            CHECK(output(4550, 711, 0) == doctest::Approx(input(4550, 711, 0)));
            CHECK(output(4550, 711, 1) == doctest::Approx(input(4550, 711, 1)));
            CHECK(output(4550, 711, 2) == doctest::Approx(input(4550, 711, 2)));
}

TEST_CASE ("testing photog_xyz_to_srgb") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> xyz =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_srgb_to_xyz(input, xyz);

    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_xyz_to_srgb(xyz, output);

    // 0.0031308f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(input(0, 0, 0)));
            CHECK(output(0, 0, 1) == doctest::Approx(input(0, 0, 1)));
            CHECK(output(0, 0, 2) == doctest::Approx(input(0, 0, 2)));
    // input(x, y, c) <= 0.0031308f
            CHECK(output(4550, 711, 0) == doctest::Approx(input(4550, 711, 0)));
            CHECK(output(4550, 711, 1) == doctest::Approx(input(4550, 711, 1)));
            CHECK(output(4550, 711, 2) == doctest::Approx(input(4550, 711, 2)));
}

TEST_CASE ("testing photog_xyz_to_rgb") {
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> xyz =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_rgb_to_xyz(input,
                      photog::get_gamma(PhotogWorkingSpace::Srgb),
                      photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb),
                      xyz);

    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_xyz_to_rgb(xyz,
                      photog::get_gamma(PhotogWorkingSpace::Srgb),
                      photog::get_xyz_to_rgb_xfmr(PhotogWorkingSpace::Srgb),
                      output);

    // 0.0031308f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(input(0, 0, 0)));
            CHECK(output(0, 0, 1) == doctest::Approx(input(0, 0, 1)));
            CHECK(output(0, 0, 2) == doctest::Approx(input(0, 0, 2)));
    // input(x, y, c) <= 0.0031308f
            CHECK(output(4550, 711, 0) == doctest::Approx(input(4550, 711, 0)));
            CHECK(output(4550, 711, 1) == doctest::Approx(input(4550, 711, 1)));
            CHECK(output(4550, 711, 2) == doctest::Approx(input(4550, 711, 2)));
}

TEST_CASE ("testing photog_average") {
    // TODO: Add test for 64-bit input.
    std::string file_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            Halide::Tools::load_and_convert_image(file_path);
    Halide::Runtime::Buffer<float> output{input.channels()};

    photog_average(input, output);

    // Reference: Kahan summation algorithm using float values
    float sums[3]{0};
    float c[3]{0};
    for (int x = 0; x < input.width(); ++x) {
        for (int y = 0; y < input.height(); ++y) {
            float z[3]{0};
            float t[3]{0};
            z[0] = input(x, y, 0) - c[0];
            z[1] = input(x, y, 1) - c[1];
            z[2] = input(x, y, 2) - c[2];
            t[0] = sums[0] + z[0];
            t[1] = sums[1] + z[1];
            t[2] = sums[2] + z[2];
            c[0] = (t[0] - sums[0]) - z[0];
            c[1] = (t[1] - sums[1]) - z[1];
            c[2] = (t[2] - sums[2]) - z[2];
            sums[0] = t[0];
            sums[1] = t[1];
            sums[2] = t[2];
        }
    }

    float averages[3]{0};
    int i{0};
    for (auto &sum:sums) {
        averages[i] = sum / input.number_of_elements();
        ++i;
    }

            CHECK(averages[0] == doctest::Approx(output(0)));
            CHECK(averages[1] == doctest::Approx(output(1)));
            CHECK(averages[2] == doctest::Approx(output(2)));
}

TEST_CASE ("testing mul_33_by_31") {
    std::array<float, 9> a_array{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
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
    std::array<float, 9> a_array{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    Halide::Runtime::Buffer<float> a{a_array.data(), {output_dim, output_dim}};
    std::array<float, 9> b_array{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
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