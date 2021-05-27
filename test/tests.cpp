# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <string>

#include "doctest/doctest.h"
#include "halide_image_io.h"
#include "HalideBuffer.h"

// Available after a CMake build
#include "photog_srgb_to_linear.h"

TEST_CASE ("testing srgb_to_linear") {
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