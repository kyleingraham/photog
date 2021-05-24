#include "Halide.h"

class SrgbToLinear : public Halide::Generator<SrgbToLinear> {
public:
    // External control of how this function is auto-scheduled
    GeneratorParam<int> x_max{"x_max", 3456};
    GeneratorParam<int> y_max{"y_max", 4608};

    Input <Func> srgb{"srgb"};
    Output <Func> linear{"linear"};

    Var x{"x"}, y{"y"}, c{"c"};

    void generate() {
        linear(x, y, c) = Halide::select(srgb(x, y, c) <= 0.04045f,
                                         srgb(x, y, c) / 12.92f,
                                         Halide::pow(
                                                 (srgb(x, y, c) + 0.055f) /
                                                 1.055f, 2.4f));
    }

    void schedule() {
        if (auto_schedule) {
            const int X{x_max}, Y{y_max}, C{3};

            srgb.set_estimate(srgb.args()[0], 0, X);
            srgb.set_estimate(srgb.args()[1], 0, Y);
            srgb.set_estimate(srgb.args()[2], 0, C);

            linear.set_estimates({{0, X},
                                  {0, Y},
                                  {0, C}});

        } else {
            // TODO: Can we make this more descriptive when it fails?
            assert(false && "Non-auto-schedule not supported.");
            abort();
        }
    }
};

// TODO: Namespace via 'photog_' prefix.
// TODO: Can we make these function part of the API?
HALIDE_REGISTER_GENERATOR(SrgbToLinear, srgb_to_linear);