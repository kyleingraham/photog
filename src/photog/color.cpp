#include "photog/color.h"

#include <array>

#include "Halide.h"

#include "color_utils.h"
#include "photog_average.h"
#include "photog_chromadapt_impl.h"
#include "utils.h"

void photog_chromadapt_diy(float *input, int width, int height,
                           float *source_tristimulus,
                           PhotogWorkingSpace working_space,
                           PhotogChromadaptMethod chromadapt_method,
                           float *dest_tristimulus, float *output) {
    const int channels = 3;
    Halide::Runtime::Buffer<float> in =
            photog::get_buffer<float>(input, width, height, channels);
    Halide::Runtime::Buffer<float> source_est(source_tristimulus, 3);
    Halide::Runtime::Buffer<float> dest(dest_tristimulus, 3);
    Halide::Runtime::Buffer<float> out =
            photog::get_buffer<float>(output, width, height, channels);

    Halide::Runtime::Buffer<float> transform =
            photog::create_transform(chromadapt_method, source_est, dest);

    photog_chromadapt_impl(in, photog::get_gamma(working_space),
                           photog::get_rgb_to_xyz_xfmr(working_space),
                           photog::get_xyz_to_rgb_xfmr(working_space),
                           transform, out);
}

void photog_chromadapt(float *input, int width, int height,
                       PhotogWorkingSpace working_space,
                       PhotogChromadaptMethod chromadapt_method,
                       PhotogIlluminant dest_illuminant, float *output) {
    // TODO: Add way to use method other gray-world.
    const int channels = 3;
    Halide::Runtime::Buffer<float> in =
            photog::get_buffer<float>(input, width, height, channels);
    Halide::Runtime::Buffer<float> source_est(3);

    photog_average(in, source_est);
    float gamma = photog::get_gamma(working_space);
    Halide::Runtime::Buffer<float> rgb_to_xyz_xfmr =
            photog::get_rgb_to_xyz_xfmr(working_space);
    source_est = photog::rgb_to_xyz(source_est, gamma, rgb_to_xyz_xfmr);

    std::array<float, 3> dest_tristimulus =
            photog::get_tristimulus(dest_illuminant);

    photog_chromadapt_diy(input, width, height, source_est.data(),
                          working_space, chromadapt_method,
                          dest_tristimulus.data(), output);
}
