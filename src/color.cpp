#include "generator.h"

class SrgbToLinear : public photog::generator<SrgbToLinear> {
public:
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

// TODO: Namespace via 'photog_' prefix.
// TODO: Can we make these function part of the API? Better to make generators available?
HALIDE_REGISTER_GENERATOR(SrgbToLinear, srgb_to_linear);