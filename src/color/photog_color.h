#ifndef PHOTOG_PHOTOG_COLOR_H
#define PHOTOG_PHOTOG_COLOR_H

#include <cmath>
#include <type_traits>

#include "Halide.h"

#include "photog_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

enum PhotogWorkingSpace {
    Srgb
};

enum PhotogChromadaptMethod {
    Bradford
};

enum PhotogIlluminant {
    D65
};

#ifdef __cplusplus
}  // extern "C"
#endif

// TODO: These should be in a private header. Possible with new C facade that uses these.
// TODO: 3 libs from one folder?: color, generators
namespace photog {
    Halide::Runtime::Buffer<float>
    create_transform(PhotogChromadaptMethod chromadapt_method,
                     const Halide::Runtime::Buffer<float> &source_tristimulus,
                     PhotogIlluminant dest_illuminant);

    Halide::Runtime::Buffer<float>
    create_tristimulus(float *tristimulus);

    float get_gamma(PhotogWorkingSpace working_space);

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

#endif //PHOTOG_PHOTOG_COLOR_H
