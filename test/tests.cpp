# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "halide_image_io.h"
#include "HalideBuffer.h"

#include "photog_color.h"
// Available after a CMake build
#include "photog_srgb_to_linear.h"
#include "photog_srgb_to_xyz.h"
#include "photog_rgb_to_xyz.h"
#include "photog_linear_to_srgb.h"
#include "photog_xyz_to_srgb.h"
#include "photog_xyz_to_rgb.h"

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
                      photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace::srgb),
                      output);

    // 0.04045f < input(x, y, c)
            CHECK(output(0, 0, 0) == doctest::Approx(0.331956f));
            CHECK(output(0, 0, 1) == doctest::Approx(0.348585f));
            CHECK(output(0, 0, 2) == doctest::Approx(0.233883f));
    // input(x, y, c) <= 0.04045f
            CHECK(output(1824, 445, 0) == doctest::Approx(0.002775f));
            CHECK(output(1824, 445, 1) == doctest::Approx(0.002991f));
            CHECK(output(1824, 445, 2) == doctest::Approx(0.002728f));
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

    photog_srgb_to_xyz(input, xyz);

    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>::make_with_shape_of(input);

    photog_xyz_to_rgb(xyz, photog_get_xyz_to_rgb_xfmr(PhotogWorkingSpace::srgb),
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