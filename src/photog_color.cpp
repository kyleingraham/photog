#include <array>
#include <map>

#include "photog_color.h"
#include "photog_generator.h"

namespace photog {
    Halide::Expr srgb_to_linear(const Halide::Expr &channel) {
        return Halide::select(channel <= 0.04045f,
                              channel / 12.92f,
                              Halide::pow((channel + 0.055f) / 1.055f,
                                          2.4f));
    }

    class SrgbToLinear : public photog::Generator<SrgbToLinear> {
    public:
        Input <Func> srgb{"srgb", Float(32), 3};
        Output <Func> linear{"linear", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            linear(x, y, c) = photog::srgb_to_linear(srgb(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            srgb.set_estimate(srgb.args()[0], 0, X);
            srgb.set_estimate(srgb.args()[1], 0, Y);
            srgb.set_estimate(srgb.args()[2], 0, C);

            linear.set_estimates({{0, X},
                                  {0, Y},
                                  {0, C}});
        }
    };
} // namespace photog

typedef std::map<PhotogWorkingSpace, std::array<float, 9>> XfmrMap;

static XfmrMap rgb_to_xyz_xfmrs{
        {PhotogWorkingSpace::srgb,
                {0.4124564f, 0.3575761f, 0.1804375f,
                        0.2126729f, 0.7151522f, 0.0721750f,
                        0.0193339f, 0.1191920f, 0.9503041f}}
};

halide_buffer_t *photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space) {
    // TODO: What are the ownership considerations here?
    return Halide::Runtime::Buffer<float>(
            rgb_to_xyz_xfmrs.at(working_space).data(), {3, 3}).raw_buffer();
}

namespace photog {
    Halide::Buffer<float>
    get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space) {
        // TODO: What are the ownership considerations here?
        return Halide::Buffer<float>(rgb_to_xyz_xfmrs.at(working_space).data(),
                                     {3, 3});
    }

    class SrgbToXyz : public photog::Generator<SrgbToXyz> {
    public:
        Input <Func> srgb{"srgb", Float(32), 3};
        Output <Func> xyz{"xyz", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            Halide::Buffer<float> rgb_to_xyz_xfmr =
                    photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::srgb);
            linear(x, y, c) = photog::srgb_to_linear(srgb(x, y, c));
            xyz(x, y, c) = rgb_to_xyz_xfmr(0, c) * linear(x, y, 0) +
                           rgb_to_xyz_xfmr(1, c) * linear(x, y, 1) +
                           rgb_to_xyz_xfmr(2, c) * linear(x, y, 2);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            srgb.set_estimate(srgb.args()[0], 0, X);
            srgb.set_estimate(srgb.args()[1], 0, Y);
            srgb.set_estimate(srgb.args()[2], 0, C);

            xyz.set_estimates({{0, X},
                               {0, Y},
                               {0, C}});
        }
    };

    class RgbToXyz : public photog::Generator<RgbToXyz> {
    public:
        Input <Func> rgb{"rgb", Float(32), 3};
        Input <Buffer<float>> rgb_to_xyz_xfmr{"rgb_to_xyz_xfmr", 2};
        Output <Func> xyz{"xyz", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            linear(x, y, c) = photog::srgb_to_linear(rgb(x, y, c));
            xyz(x, y, c) = rgb_to_xyz_xfmr(0, c) * linear(x, y, 0) +
                           rgb_to_xyz_xfmr(1, c) * linear(x, y, 1) +
                           rgb_to_xyz_xfmr(2, c) * linear(x, y, 2);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            rgb.set_estimate(rgb.args()[0], 0, X);
            rgb.set_estimate(rgb.args()[1], 0, Y);
            rgb.set_estimate(rgb.args()[2], 0, C);

            xyz.set_estimates({{0, X},
                               {0, Y},
                               {0, C}});
        }
    };
} // namespace photog

// TODO: Can we make these functions part of the API? Better to make generators available?
// TODO: What is the third argument used for? Stubs and Generator composing?
HALIDE_REGISTER_GENERATOR(photog::SrgbToLinear, photog_srgb_to_linear);
HALIDE_REGISTER_GENERATOR(photog::SrgbToXyz, photog_srgb_to_xyz);
HALIDE_REGISTER_GENERATOR(photog::RgbToXyz, photog_rgb_to_xyz);