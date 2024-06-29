# define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <array>
#include <iostream>

#include "doctest/doctest.h"
#include "Halide.h"
#include "halide_image_io.h"

#include "photog/color.h"
#include "color_utils.h"
#include "utils.h"
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
#include "photog_zero_mask.h"
#include "photog_toroidal_histogram.h"

namespace photog {
    template<typename T>
    Halide::Runtime::Buffer<T> load_image(const std::string &image_path) {
        photog::Layout layout = photog::get_layout();

        Halide::Runtime::Buffer<T> image =
                Halide::Tools::load_and_convert_image(image_path);

        if (layout == Layout::Planar)
            return image;

        if (layout == Layout::Interleaved)
            return image.copy_to_interleaved();

        std::cerr << "Unsupported image layout " << static_cast<int>(layout)
                << " in photog::load_image()." << std::endl;
        abort();
    }

    template<typename T>
    Halide::Runtime::Buffer<T> get_buffer(int width, int height, int channels) {
        photog::Layout layout = photog::get_layout();

        if (layout == Layout::Planar)
            return Halide::Runtime::Buffer<T>{width, height, channels};

        if (layout == Layout::Interleaved)
            return Halide::Runtime::Buffer<T>::make_interleaved(width, height, channels);

        std::cerr << "Unsupported image layout " << static_cast<int>(layout)
                << " in photog::get_buffer()." << std::endl;
        abort();
    }
}

template<typename T>
struct Stats {
    T min = 0;
    T max = 0;
    T sum = 0;
};

/**
 * Get min, max, and sum values for a 2D Halide buffer.
 */
template<typename T>
Stats<T> get_stats(const Halide::Runtime::Buffer<T> &buffer) {
    T value;
    Stats<T> stats;

    for (int i = 0; i < buffer.width(); ++i) {
        for (int j = 0; j < buffer.height(); ++j) {
            value = buffer(i, j);
            stats.sum += value;

            if (value < stats.min) {
                stats.min = value;
            }

            if (stats.max < value) {
                stats.max = value;
            }
        }
    }

    return stats;
}

TEST_CASE("testing photog_zero_mask") {
    std::string image_path = R"(images/cheng/preprocessed/Cheng/Canon1DsMkIII/000001.png)";
    Halide::Runtime::Buffer<float> input = photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<int> mask = Halide::Runtime::Buffer<int>{input.width(), input.height()};

    photog_zero_mask(input, mask);
    auto [min, max, sum] = get_stats(mask);
    CHECK_EQ(min, 0);
    CHECK_EQ(max, 1);
    // Do we have the same number of masked pixels?
    CHECK_EQ(sum, 93761);
    // Spot checks
    CHECK_EQ(mask(82, 83), 0);
    CHECK_EQ(mask(0, 0), 1);

    /*
     * To generate test data run the following in MATLAB using the Google ffcc repo:
     *
     *   cd('ffcc'); % Replace 'ffcc' with where you have downloaded the repo
     *   addpath(genpath('.'));
     *   img = imread('./data/cheng/preprocessed/Cheng/Canon1DsMkIII/000001.png');
     *   I_valid = all(img > 0,3)
     *   % Sum
     *   mask_sum = sum(I_valid(:))
     *   % Non-zero value coordinates
     *   [c1, c2] = ind2sub(size(I_valid(:,:)), find(I_valid(:,:), 1, "first"))
     *   % Use image editor to find zero region then verify in MATLAB
     *   img(83,84) == 0
     *   I_valid(83,84) == 0
     */
}

TEST_CASE("testing photog_toroidal_histogram") {
    std::string image_path = R"(images/cheng/preprocessed/Cheng/Canon1DsMkIII/000001.png)";
    Halide::Runtime::Buffer<float> input = photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<int> mask = Halide::Runtime::Buffer<int>{input.width(), input.height()};
    Halide::Runtime::Buffer<float> output =
            Halide::Runtime::Buffer<float>{photog::histogram_bin_count, photog::histogram_bin_count};

    mask.fill(1);
    photog_toroidal_histogram(input, mask, output);

    auto [min, max, sum] = get_stats(output);

    // Is the histogram normalized?
    CHECK(sum == doctest::Approx(1.0f));
    // Do we match the Google ffcc reference?
    CHECK(min == doctest::Approx(0.0f));
    CHECK(max == doctest::Approx(0.220017f));
    CHECK(output(29, 36) == doctest::Approx(1.06654e-05f)); // Spot check
    CHECK(output(0, 0) == doctest::Approx(min));
    CHECK(output(23, 46) == doctest::Approx(max));

    /*
     * To generate test data run the following in MATLAB using the Google ffcc repo:
     *
     *   cd('ffcc'); % Replace 'ffcc' with where you have downloaded the repo
     *   addpath(genpath('.'));
     *   params = LoadProjectParams('ChengCanon1DsMkIII');
     *   img = imread('./data/cheng/preprocessed/Cheng/Canon1DsMkIII/000001.png');
     *   img_valid = true(size(img, 1), size(img, 2))
     *   [X, img_channels] = FeaturizeImage(img, img_valid, params);
     *   % Spot check value and coordinates
     *   [c1, c2, c3] = ind2sub(size(X(:,:,1)), find(X(:,:,1), 1, "first"))
     *   spot_check_val = X(c1, c2, c3)
     *   X1 = X(:,:,1)
     *   % Maximum value and coordinates
     *   [max_val, linear_coord] = max(X1(:))
     *   [max_x, max_y] = ind2sub(size(X1), linear_coord);
     *   % Minimum value and coordinates
     *   [min_val, linear_coord] = min(X1(:))
     *   [min_x, min_y] = ind2sub(size(X1), linear_coord);
     */
}

TEST_CASE("testing photog_srgb_to_linear") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_chromadapt") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());
    Halide::Runtime::Buffer<float> source_tristimulus(3);

    photog_chromadapt(input.data(), input.width(), input.height(),
                      PhotogWorkingSpace::Srgb,
                      PhotogChromadaptMethod::Bradford,
                      PhotogIlluminant::D50,
                      output.data());

    Halide::Tools::convert_and_save_image(output, R"(images/out.jpg)");
}

TEST_CASE("testing photog_chromadapt_diy") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());
    Halide::Runtime::Buffer<float> source_tristimulus(3);

    // Pixel that should be white in the destination image.
    for (int i = 0; i < 3; ++i)
        source_tristimulus(i) = input(328, 3261, i);

    source_tristimulus =
            photog::rgb_to_xyz(source_tristimulus,
                               photog::get_gamma(PhotogWorkingSpace::Srgb),
                               photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb));

    photog_chromadapt_diy(input.data(), input.width(), input.height(),
                          source_tristimulus.data(),
                          PhotogWorkingSpace::Srgb,
                          PhotogChromadaptMethod::Bradford,
                          photog::get_tristimulus(PhotogIlluminant::D50).data(),
                          output.data());

    Halide::Tools::convert_and_save_image(output, R"(images/out.jpg)");
}

TEST_CASE("testing photog_rgb_to_linear") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_srgb_to_xyz") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_rgb_to_xyz") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_linear_to_srgb") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> linear =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

    photog_srgb_to_linear(input, linear);

    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_linear_to_rgb") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> linear =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

    photog_rgb_to_linear(input, photog::get_gamma(PhotogWorkingSpace::Srgb),
                         linear);

    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_xyz_to_srgb") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> xyz =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

    photog_srgb_to_xyz(input, xyz);

    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_xyz_to_rgb") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
    Halide::Runtime::Buffer<float> xyz =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

    photog_rgb_to_xyz(input,
                      photog::get_gamma(PhotogWorkingSpace::Srgb),
                      photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb),
                      xyz);

    Halide::Runtime::Buffer<float> output =
            photog::get_buffer<float>(input.width(), input.height(),
                                      input.channels());

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

TEST_CASE("testing photog_average") {
    std::string image_path = R"(images/rgb.jpg)";
    Halide::Runtime::Buffer<float> input =
            photog::load_image<float>(image_path);
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
    for (auto &sum: sums) {
        averages[i] = sum / input.number_of_elements();
        ++i;
    }

    CHECK(averages[0] == doctest::Approx(output(0)));
    CHECK(averages[1] == doctest::Approx(output(1)));
    CHECK(averages[2] == doctest::Approx(output(2)));
}
