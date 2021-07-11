#include "color_utils.h"

#include <algorithm>
#include <array>
#include <map>

#include "Halide.h"

#include "photog/color.h"
#include "utils.h"

namespace photog {
    Halide::Runtime::Buffer<float> create_tristimulus(float *tristimulus) {
        const int expected_dim = 3;
        std::array<float, expected_dim> temp{};
        // We should be safe here since we never write to the resultant buffer.
        std::copy(tristimulus, tristimulus + expected_dim, temp.begin());
        return copy_to_buffer(temp);
    }

    float get_gamma(PhotogWorkingSpace working_space) {
        static std::map<PhotogWorkingSpace, float> gammas =
                {{PhotogWorkingSpace::Srgb, 2.2f}};

        return gammas.at(working_space);
    }

    std::array<float, 3>
    get_tristimulus(PhotogIlluminant illuminant) {
        static std::map<PhotogIlluminant, std::array<float, 3>> tristimuli =
                {{PhotogIlluminant::D65, {0.95047f, 1.0f, 1.08883f}}};

        return tristimuli.at(illuminant);
    }

    Halide::Runtime::Buffer<float>
    get_xyz_to_lms_xfmr(PhotogChromadaptMethod chromadapt_method) {
        static std::map<int, std::array<float, 9>> xfmrs =
                {{PhotogChromadaptMethod::Bradford, {0.8951f, 0.2664f, -0.1614f,
                                                            -0.7502f, 1.7135f, 0.0367f,
                                                            0.0389f, -0.0685f, 1.0296f}}};

        return copy_to_buffer(xfmrs.at(chromadapt_method), 3);
    }

    Halide::Runtime::Buffer<float>
    get_lms_to_xyz_xfmr(PhotogChromadaptMethod chromadapt_method) {
        static std::map<int, std::array<float, 9>> xfmrs =
                {{PhotogChromadaptMethod::Bradford, {0.9869929f, -0.1470543f, 0.1599627f,
                                                            0.4323053f, 0.5183603f, 0.0492912f,
                                                            -0.0085287f, 0.0400428f, 0.9684867f}}};

        return copy_to_buffer(xfmrs.at(chromadapt_method), 3);
    }

    Halide::Runtime::Buffer<float>
    get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space) {
        static std::map<int, std::array<float, 9>> xfmrs =
                {{PhotogWorkingSpace::Srgb, {0.4124564f, 0.3575761f, 0.1804375f,
                                                    0.2126729f, 0.7151522f, 0.0721750f,
                                                    0.0193339f, 0.1191920f, 0.9503041f}}};

        return copy_to_buffer(xfmrs.at(working_space), 3);
    }

    Halide::Runtime::Buffer<float>
    get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space) {
        static std::map<int, std::array<float, 9>> xfmrs =
                {{PhotogWorkingSpace::Srgb, {3.2404542f, -1.5371385f, -0.4985314f,
                                                    -0.9692660f, 1.8760108f, 0.0415560f,
                                                    0.0556434f, -0.2040259f, 1.0572252f}}};

        return copy_to_buffer(xfmrs.at(working_space), 3);
    }

    template<typename T>
    Halide::Runtime::Buffer<T>
    normalize_y(const Halide::Runtime::Buffer<T> &xyz_tristimulus, T to) {
        static_assert(std::is_floating_point<T>::value,
                      "Function only supports floating-point buffers.");
        const int output_dim = 3;
        assert(xyz_tristimulus.dimensions() == 1);
        assert(xyz_tristimulus.dim(0).extent() == output_dim);
        T factor = to / xyz_tristimulus(1);
        Halide::Runtime::Buffer<T> output(output_dim);
        for (int i = 0; i < output_dim; ++i)
            output(i) = xyz_tristimulus(i) * factor;

        return output;
    }

    Halide::Runtime::Buffer<float>
    create_transform(PhotogChromadaptMethod chromadapt_method,
                     const Halide::Runtime::Buffer<float> &source_tristimulus,
                     const Halide::Runtime::Buffer<float> &dest_tristimulus) {
        auto xyz_to_lms_xfmr =
                photog::get_xyz_to_lms_xfmr(chromadapt_method);
        auto lms_to_xyz_xfmr =
                photog::get_lms_to_xyz_xfmr(chromadapt_method);

        auto lms_source =
                photog::mul_33_by_31(xyz_to_lms_xfmr,
                                     photog::normalize_y(source_tristimulus,
                                                         100.0f));
        auto lms_dest =
                photog::mul_33_by_31(xyz_to_lms_xfmr,
                                     photog::normalize_y(dest_tristimulus,
                                                         100.0f));
        auto lms_gain =
                photog::div_vec_by_vec(lms_dest, lms_source);
        auto lms_gain_diagonal =
                photog::create_diagonal(lms_gain);
        auto transform_temp =
                photog::mul_33_by_33(lms_to_xyz_xfmr, lms_gain_diagonal);
        auto transform =
                photog::mul_33_by_33(transform_temp, xyz_to_lms_xfmr);

        return transform;
    }
}