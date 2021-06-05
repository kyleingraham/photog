#include <array>
#include <map>

#include "photog_color.h"
#include "photog_generator.h"

typedef std::map<PhotogWorkingSpace, std::array<float, 9>> XfmrMap;
typedef std::map<PhotogWorkingSpace, float> GammaMap;

namespace photog {
    /** Calculates average pixel value for an image.*/
    Halide::Func
    average(const Halide::Func &image, const Halide::Type &image_type,
            const Halide::Expr &width, const Halide::Expr &height,
            const Halide::Expr &channels) {
        Halide::Func average{"func_average"}, sum{"func_sum"};
        Halide::Var c{"func_c"};
        Halide::RDom r{0, width, 0, height};
        Halide::Type narrow = image_type;
        Halide::Type wide;

        if (image_type.bits() != 64)
            wide = image_type.widen();
        else
            wide = narrow;

        sum(c) = Halide::cast(wide, 0);
        sum(c) = Halide::sum(Halide::cast(wide, image(r.x, r.y, c)));
        average(c) = Halide::cast(narrow, sum(c) / (width * height * channels));
        return average;
    }

    class Average : public photog::Generator<Average> {
    public:
        // TODO: How do we vary type for testing?
        Input <Buffer<float>> input{"input", 3};
        Output <Buffer<float>> average{"average", 1};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            // TODO: Auto-scheduling limits to a single core here. Use atomic to parallelize?
            average(c) = photog::average(input, input.type(), input.width(),
                                         input.height(), input.channels())(c);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            input.set_estimates({{0, X},
                                 {0, Y},
                                 {0, C}});

            average.set_estimates({{0, C}});
        }
    };

    Halide::Expr srgb_to_linear(const Halide::Expr &channel) {
        return Halide::select(channel <= 0.04045f,
                              channel / 12.92f,
                              Halide::pow((channel + 0.055f) / 1.055f,
                                          2.4f));
    }

    class SrgbToLinear : public photog::Generator<SrgbToLinear> {
    public:
        // TODO: Can we avoid explicit typing here?
        // TODO: How do we handle 4-channel images?
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

    Halide::Expr linear_to_srgb(const Halide::Expr &channel) {
        return Halide::select(channel <= 0.0031308f,
                              channel * 12.92f,
                              (1.055f * Halide::pow(channel,
                                                    1 / 2.4f)) - 0.055f);
    }

    class LinearToSrgb : public photog::Generator<LinearToSrgb> {
    public:
        Input <Func> linear{"linear", Float(32), 3};
        Output <Func> srgb{"srgb", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            srgb(x, y, c) = photog::linear_to_srgb(linear(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            linear.set_estimate(linear.args()[0], 0, X);
            linear.set_estimate(linear.args()[1], 0, Y);
            linear.set_estimate(linear.args()[2], 0, C);

            srgb.set_estimates({{0, X},
                                {0, Y},
                                {0, C}});
        }
    };

    Halide::Expr
    rgb_to_linear(const Halide::Expr &channel, const Halide::Expr &gamma) {
        return Halide::pow(channel, gamma);
    }

    class RgbToLinear : public photog::Generator<RgbToLinear> {
    public:
        Input <Func> rgb{"rgb", Float(32), 3};
        Input<float> gamma{"gamma"};
        Output <Func> linear{"linear", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            linear(x, y, c) = photog::rgb_to_linear(rgb(x, y, c), gamma);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            rgb.set_estimate(rgb.args()[0], 0, X);
            rgb.set_estimate(rgb.args()[1], 0, Y);
            rgb.set_estimate(rgb.args()[2], 0, C);

            gamma.set_estimate(2.2);

            linear.set_estimates({{0, X},
                                  {0, Y},
                                  {0, C}});
        }
    };

    Halide::Expr
    linear_to_rgb(const Halide::Expr &channel, const Halide::Expr &gamma) {
        return Halide::pow(channel, 1 / gamma);
    }

    class LinearToRgb : public photog::Generator<LinearToRgb> {
    public:
        Input <Func> linear{"linear", Float(32), 3};
        Input<float> gamma{"gamma"};
        Output <Func> rgb{"rgb", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            rgb(x, y, c) = photog::linear_to_rgb(linear(x, y, c), gamma);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            linear.set_estimate(linear.args()[0], 0, X);
            linear.set_estimate(linear.args()[1], 0, Y);
            linear.set_estimate(linear.args()[2], 0, C);

            gamma.set_estimate(2.2);

            rgb.set_estimates({{0, X},
                               {0, Y},
                               {0, C}});
        }
    };

    GammaMap working_space_gammas{{PhotogWorkingSpace::srgb, 2.2}};
} // namespace photog

float photog_get_working_space_gamma(PhotogWorkingSpace working_space) {
    return photog::working_space_gammas.at(working_space);
}

namespace photog {
    static XfmrMap rgb_to_xyz_xfmrs{
            {PhotogWorkingSpace::srgb,
                    {0.4124564f, 0.3575761f, 0.1804375f,
                            0.2126729f, 0.7151522f, 0.0721750f,
                            0.0193339f, 0.1191920f, 0.9503041f}}
    };

    static XfmrMap xyz_to_rgb_xfmrs{
            {PhotogWorkingSpace::srgb,
                    {3.2404542f, -1.5371385f, -0.4985314f,
                            -0.9692660f, 1.8760108f, 0.0415560f,
                            0.0556434f, -0.2040259f, 1.0572252f}}
    };

    Halide::Runtime::Buffer<float>
    get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space) {
        // TODO: What are the ownership considerations here? Copied by value?
        return Halide::Runtime::Buffer<float>(
                photog::rgb_to_xyz_xfmrs.at(working_space).data(),
                {3, 3});
    }

    Halide::Runtime::Buffer<float>
    get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space) {
        // TODO: What are the ownership considerations here? Copied by value?
        return Halide::Runtime::Buffer<float>(
                photog::xyz_to_rgb_xfmrs.at(working_space).data(),
                {3, 3});
    }
} // namespace photog

halide_buffer_t *photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space) {
    // TODO: What are the ownership considerations here? Copied by value?
    return photog::get_rgb_to_xyz_xfmr(working_space).raw_buffer();
}

halide_buffer_t *photog_get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space) {
    // TODO: What are the ownership considerations here? Copied by value?
    return photog::get_xyz_to_rgb_xfmr(working_space).raw_buffer();
}

namespace photog {
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
        Input<float> gamma{"gamma"};
        Input <Buffer<float>> rgb_to_xyz_xfmr{"xyz_to_rgb_xfmr", 2};
        Output <Func> xyz{"xyz", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            linear(x, y, c) = photog::rgb_to_linear(rgb(x, y, c), gamma);
            xyz(x, y, c) = rgb_to_xyz_xfmr(0, c) * linear(x, y, 0) +
                           rgb_to_xyz_xfmr(1, c) * linear(x, y, 1) +
                           rgb_to_xyz_xfmr(2, c) * linear(x, y, 2);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            rgb.set_estimate(rgb.args()[0], 0, X);
            rgb.set_estimate(rgb.args()[1], 0, Y);
            rgb.set_estimate(rgb.args()[2], 0, C);

            gamma.set_estimate(2.2);

            xyz.set_estimates({{0, X},
                               {0, Y},
                               {0, C}});
        }
    };

    class XyzToSrgb : public photog::Generator<XyzToSrgb> {
    public:
        Input <Func> xyz{"xyz", Float(32), 3};
        Output <Func> srgb{"srgb", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            Halide::Buffer<float> xyz_to_rgb_xfmr =
                    photog::get_xyz_to_rgb_xfmr(PhotogWorkingSpace::srgb);
            linear(x, y, c) = xyz_to_rgb_xfmr(0, c) * xyz(x, y, 0) +
                              xyz_to_rgb_xfmr(1, c) * xyz(x, y, 1) +
                              xyz_to_rgb_xfmr(2, c) * xyz(x, y, 2);
            srgb(x, y, c) = photog::linear_to_srgb(linear(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            xyz.set_estimate(xyz.args()[0], 0, X);
            xyz.set_estimate(xyz.args()[1], 0, Y);
            xyz.set_estimate(xyz.args()[2], 0, C);

            srgb.set_estimates({{0, X},
                                {0, Y},
                                {0, C}});
        }
    };

    class XyzToRgb : public photog::Generator<XyzToRgb> {
    public:
        Input <Func> xyz{"xyz", Float(32), 3};
        Input<float> gamma{"gamma"};
        Input <Buffer<float>> xyz_to_rgb_xfmr{"xyz_to_rgb_xfmr", 2};
        Output <Func> rgb{"rgb", Float(32), 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            linear(x, y, c) = xyz_to_rgb_xfmr(0, c) * xyz(x, y, 0) +
                              xyz_to_rgb_xfmr(1, c) * xyz(x, y, 1) +
                              xyz_to_rgb_xfmr(2, c) * xyz(x, y, 2);
            rgb(x, y, c) = photog::linear_to_rgb(linear(x, y, c), gamma);
        }

        void schedule_auto() override {
            const int X{x_max}, Y{y_max}, C{3};

            xyz.set_estimate(xyz.args()[0], 0, X);
            xyz.set_estimate(xyz.args()[1], 0, Y);
            xyz.set_estimate(xyz.args()[2], 0, C);

            gamma.set_estimate(2.2);

            rgb.set_estimates({{0, X},
                               {0, Y},
                               {0, C}});
        }
    };
} // namespace photog

// TODO: What is the third argument used for? Stubs and Generator composing?
HALIDE_REGISTER_GENERATOR(photog::SrgbToLinear, photog_srgb_to_linear);
HALIDE_REGISTER_GENERATOR(photog::RgbToLinear, photog_rgb_to_linear);
HALIDE_REGISTER_GENERATOR(photog::SrgbToXyz, photog_srgb_to_xyz);
HALIDE_REGISTER_GENERATOR(photog::RgbToXyz, photog_rgb_to_xyz);
HALIDE_REGISTER_GENERATOR(photog::LinearToSrgb, photog_linear_to_srgb);
HALIDE_REGISTER_GENERATOR(photog::LinearToRgb, photog_linear_to_rgb);
HALIDE_REGISTER_GENERATOR(photog::XyzToSrgb, photog_xyz_to_srgb);
HALIDE_REGISTER_GENERATOR(photog::XyzToRgb, photog_xyz_to_rgb);
HALIDE_REGISTER_GENERATOR(photog::Average, photog_average);