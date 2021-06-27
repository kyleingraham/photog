#ifndef PHOTOG_COLOR_UTILS_H
#define PHOTOG_COLOR_UTILS_H

#include <cmath>
#include <type_traits>

#include "Halide.h"

#include "photog/color.h"
#include "utils.h"

namespace photog {
    Halide::Runtime::Buffer<float>
    create_transform(PhotogChromadaptMethod chromadapt_method,
                     const Halide::Runtime::Buffer<float> &source_tristimulus,
                     const Halide::Runtime::Buffer<float> &dest_tristimulus);

    float get_gamma(PhotogWorkingSpace working_space);

    std::array<float, 3>
    get_tristimulus(PhotogIlluminant illuminant);

    Halide::Runtime::Buffer<float>
    get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space);

    Halide::Runtime::Buffer<float>
    get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space);

    template<typename T>
    Halide::Runtime::Buffer<T>
    rgb_to_xyz(const Halide::Runtime::Buffer<T> &rgb, float gamma,
               const Halide::Runtime::Buffer<T> &rgb_to_xyz_xfmr) {
        static_assert(std::is_floating_point<T>::value,
                      "Function only valid for floating-point buffers.");
        static int output_dim = 3; // 3-channel image
        Halide::Runtime::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i)
            output(i) = (T) std::pow(rgb(i), gamma);

        output = mul_33_by_31(rgb_to_xyz_xfmr, output);

        return output;
    }
}

#endif //PHOTOG_COLOR_UTILS_H
