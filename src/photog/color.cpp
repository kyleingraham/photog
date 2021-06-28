#include "photog/color.h"

#include <array>

#include "Halide.h"

#include "color_utils.h"
#include "photog_average.h"
#include "photog_chromadapt_p3_impl.h"

void photog_chromadapt_diy_p3(float *input, int width, int height,
                              PhotogWorkingSpace working_space,
                              PhotogChromadaptMethod chromadapt_method,
                              float *dest_tristimulus, float *output) {
    const int channels = 3;
    Halide::Runtime::Buffer<float> in{input, {width, height, channels}};
    Halide::Runtime::Buffer<float> source_est(3);
    Halide::Runtime::Buffer<float> dest(dest_tristimulus, 3);
    Halide::Runtime::Buffer<float> out{output, {width, height, channels}};

    photog_average(in, source_est);

    float gamma = photog::get_gamma(working_space);
    Halide::Runtime::Buffer<float> rgb_to_xyz_xfmr =
            photog::get_rgb_to_xyz_xfmr(working_space);

    source_est = photog::rgb_to_xyz(source_est, gamma, rgb_to_xyz_xfmr);

    Halide::Runtime::Buffer<float> transform =
            photog::create_transform(chromadapt_method, source_est, dest);

    photog_chromadapt_p3_impl(in, gamma, rgb_to_xyz_xfmr,
                              photog::get_xyz_to_rgb_xfmr(working_space),
                              transform, out);
}

void photog_chromadapt_p3(float *input, int width, int height,
                          PhotogWorkingSpace working_space,
                          PhotogChromadaptMethod chromadapt_method,
                          PhotogIlluminant dest_illuminant, float *output) {
    std::array<float, 3> dest_tristimulus =
            photog::get_tristimulus(dest_illuminant);

    photog_chromadapt_diy_p3(input, width, height, working_space,
                             chromadapt_method, dest_tristimulus.data(),
                             output);
}