#include "Halide.h"

#include "photog/color.h"
#include "color_utils.h"
#include "constants.h"
#include "generator.h"

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

        // Refactoring this code to remove intermediate Func sum does not yield a speed increase.
        sum(c) = Halide::cast(wide, 0);
        sum(c) = Halide::sum(Halide::cast(wide, image(r.x, r.y, c)));
        average(c) = Halide::cast(narrow, sum(c) / (width * height * channels));

        return average;
    }

    class Average : public photog::Generator<Average> {
    public:
        Input<Buffer<float> > input{"input", 3};
        Output<Buffer<float> > average{"average", 1};

        Var c{"c"};

        void generate() {
            average(c) = photog::average(input, input.type(), input.width(),
                                         input.height(), input.channels())(c);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            input.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            average.set_estimates({{0, C}});

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                input.dim(0).set_stride(C);
                input.dim(2).set_stride(1);
            }
        }
    };

    Halide::Expr srgb_to_linear(const Halide::Expr &channel) {
        return Halide::select(channel <= 0.04045f,
                              channel / 12.92f,
                              Halide::pow((channel + 0.055f) / 1.055f,
                                          2.4f));
    }

    class SrgbToLinear : public Generator<SrgbToLinear> {
    public:
        Input<Buffer<float> > srgb{"srgb", 3};
        Output<Buffer<float> > linear{"linear", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            linear(x, y, c) = srgb_to_linear(srgb(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            srgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            linear.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                srgb.dim(0).set_stride(C);
                srgb.dim(2).set_stride(1);
                linear.dim(0).set_stride(C);
                linear.dim(2).set_stride(1);
            }
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
        Input<Buffer<float> > linear{"linear", 3};
        Output<Buffer<float> > srgb{"srgb", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            srgb(x, y, c) = photog::linear_to_srgb(linear(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            linear.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            srgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                linear.dim(0).set_stride(C);
                linear.dim(2).set_stride(1);
                srgb.dim(0).set_stride(C);
                srgb.dim(2).set_stride(1);
            }
        }
    };

    Halide::Expr
    rgb_to_linear(const Halide::Expr &channel, const Halide::Expr &gamma) {
        return Halide::pow(channel, gamma);
    }

    class RgbToLinear : public photog::Generator<RgbToLinear> {
    public:
        Input<Buffer<float> > rgb{"rgb", 3};
        Input<float> gamma{"gamma"};
        Output<Buffer<float> > linear{"linear", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            linear(x, y, c) = photog::rgb_to_linear(rgb(x, y, c), gamma);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            rgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            gamma.set_estimate(2.2);

            linear.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                rgb.dim(0).set_stride(C);
                rgb.dim(2).set_stride(1);
                linear.dim(0).set_stride(C);
                linear.dim(2).set_stride(1);
            }
        }
    };

    Halide::Expr
    linear_to_rgb(const Halide::Expr &channel, const Halide::Expr &gamma) {
        return Halide::pow(channel, 1 / gamma);
    }

    class LinearToRgb : public photog::Generator<LinearToRgb> {
    public:
        Input<Buffer<float> > linear{"linear", 3};
        Input<float> gamma{"gamma"};
        Output<Buffer<float> > rgb{"rgb", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            rgb(x, y, c) = photog::linear_to_rgb(linear(x, y, c), gamma);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            linear.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            gamma.set_estimate(2.2);

            rgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                linear.dim(0).set_stride(C);
                linear.dim(2).set_stride(1);
                rgb.dim(0).set_stride(C);
                rgb.dim(2).set_stride(1);
            }
        }
    };

    class SrgbToXyz : public photog::Generator<SrgbToXyz> {
    public:
        Input<Buffer<float> > srgb{"srgb", 3};
        Output<Buffer<float> > xyz{"xyz", 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            Halide::Buffer<float> rgb_to_xyz_xfmr =
                    photog::get_rgb_to_xyz_xfmr(PhotogWorkingSpace::Srgb);
            linear(x, y, c) = photog::srgb_to_linear(srgb(x, y, c));
            xyz(x, y, c) = rgb_to_xyz_xfmr(0, c) * linear(x, y, 0) +
                           rgb_to_xyz_xfmr(1, c) * linear(x, y, 1) +
                           rgb_to_xyz_xfmr(2, c) * linear(x, y, 2);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            srgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            xyz.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                srgb.dim(0).set_stride(C);
                srgb.dim(2).set_stride(1);
                xyz.dim(0).set_stride(C);
                xyz.dim(2).set_stride(1);
            }
        }
    };

    Halide::Func
    rgb_to_xyz(const Halide::Func &rgb, const Halide::Expr &gamma,
               const Halide::Func &rgb_to_xyz_xfmr) {
        Halide::Func linear{"linear"}, xyz{"xyz"};
        Halide::Var x{"x"}, y{"y"}, c{"c"};

        linear(x, y, c) = photog::rgb_to_linear(rgb(x, y, c), gamma);
        xyz(x, y, c) = rgb_to_xyz_xfmr(0, c) * linear(x, y, 0) +
                       rgb_to_xyz_xfmr(1, c) * linear(x, y, 1) +
                       rgb_to_xyz_xfmr(2, c) * linear(x, y, 2);

        return xyz;
    }

    class RgbToXyz : public photog::Generator<RgbToXyz> {
    public:
        Input<Buffer<float> > rgb{"rgb", 3};
        Input<float> gamma{"gamma"};
        Input<Buffer<float> > rgb_to_xyz_xfmr{"xyz_to_rgb_xfmr", 2};
        Output<Buffer<float> > xyz{"xyz", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            xyz(x, y, c) =
                    photog::rgb_to_xyz(rgb, gamma, rgb_to_xyz_xfmr)(x, y, c);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            rgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            gamma.set_estimate(2.2);

            xyz.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                rgb.dim(0).set_stride(C);
                rgb.dim(2).set_stride(1);
                xyz.dim(0).set_stride(C);
                xyz.dim(2).set_stride(1);
            }
        }
    };

    class XyzToSrgb : public photog::Generator<XyzToSrgb> {
    public:
        Input<Buffer<float> > xyz{"xyz", 3};
        Output<Buffer<float> > srgb{"srgb", 3};

        Var x{"x"}, y{"y"}, c{"c"};
        Func linear{"linear"};

        void generate() {
            Halide::Buffer<float> xyz_to_rgb_xfmr =
                    photog::get_xyz_to_rgb_xfmr(PhotogWorkingSpace::Srgb);
            linear(x, y, c) = xyz_to_rgb_xfmr(0, c) * xyz(x, y, 0) +
                              xyz_to_rgb_xfmr(1, c) * xyz(x, y, 1) +
                              xyz_to_rgb_xfmr(2, c) * xyz(x, y, 2);
            srgb(x, y, c) = photog::linear_to_srgb(linear(x, y, c));
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            xyz.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            srgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                xyz.dim(0).set_stride(C);
                xyz.dim(2).set_stride(1);
                srgb.dim(0).set_stride(C);
                srgb.dim(2).set_stride(1);
            }
        }
    };

    Halide::Func
    xyz_to_rgb(const Halide::Func &xyz, const Halide::Expr &gamma,
               const Halide::Func &xyz_to_rgb_xfmr) {
        Halide::Func linear{"linear"}, rgb{"rgb"};
        Halide::Var x{"x"}, y{"y"}, c{"c"};

        linear(x, y, c) = xyz_to_rgb_xfmr(0, c) * xyz(x, y, 0) +
                          xyz_to_rgb_xfmr(1, c) * xyz(x, y, 1) +
                          xyz_to_rgb_xfmr(2, c) * xyz(x, y, 2);

        rgb(x, y, c) = Halide::clamp(linear_to_rgb(linear(x, y, c), gamma),
                                     0.0f, 1.0f);

        return rgb;
    }

    class XyzToRgb : public photog::Generator<XyzToRgb> {
    public:
        Input<Buffer<float> > xyz{"xyz", 3};
        Input<float> gamma{"gamma"};
        Input<Buffer<float> > xyz_to_rgb_xfmr{"xyz_to_rgb_xfmr", 2};
        Output<Buffer<float> > rgb{"rgb", 3};

        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            rgb(x, y, c) =
                    photog::xyz_to_rgb(xyz, gamma, xyz_to_rgb_xfmr)(x, y, c);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            xyz.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            gamma.set_estimate(2.2);

            rgb.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                xyz.dim(0).set_stride(C);
                xyz.dim(2).set_stride(1);
                rgb.dim(0).set_stride(C);
                rgb.dim(2).set_stride(1);
            }
        }
    };

    class Chromadapt : public photog::Generator<Chromadapt> {
    public:
        Input<Buffer<float> > input{"input", 3};
        Input<float> gamma{"gamma"};
        Input<Buffer<float> > rgb_to_xyz_xfmr{"rgb_to_xyz_xfmr", 2};
        Input<Buffer<float> > xyz_to_rgb_xfmr{"xyz_to_rgb_xfmr", 2};
        Input<Buffer<float> > transform{"transform", 2};
        Output<Buffer<float> > output{"output", 3};

        Func adapted{"adapted"}, xyz{"xyz"};
        Var x{"x"}, y{"y"}, c{"c"};

        void generate() {
            xyz(x, y, c) =
                    photog::rgb_to_xyz(input, gamma, rgb_to_xyz_xfmr)(x, y, c);

            adapted(x, y, c) =
                    transform(0, c) * xyz(x, y, 0) +
                    transform(1, c) * xyz(x, y, 1) +
                    transform(2, c) * xyz(x, y, 2);

            output(x, y, c) =
                    photog::xyz_to_rgb(adapted, gamma,
                                       xyz_to_rgb_xfmr)(x, y, c);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            input.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            gamma.set_estimate(2.2);

            output.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                input.dim(0).set_stride(C);
                input.dim(2).set_stride(1);
                output.dim(0).set_stride(C);
                output.dim(2).set_stride(1);
            }
        }
    };

    /**
     * Mask the zero-pixels in a given 3-channel image. Zero-pixels are zero in all channels.
     */
    Halide::Func zero_mask(const Halide::Func &image) {
        Halide::Var x{"x_zero_mask"}, y{"y_zero_mask"};
        Halide::Func mask{"mask_zero_mask"};
        mask(x, y) = Halide::select(
            0 < image(x, y, 0) &&
            0 < image(x, y, 1) &&
            0 < image(x, y, 2),
            1,
            0
        );
        return mask;
    }

    class ZeroMask : public Generator<ZeroMask> {
    public:
        Input<Buffer<float> > input{"input_ZeroMask", 3};
        Output<Buffer<int> > output{"output_ZeroMask", 2};

        void generate() {
            output(x, y) = zero_mask(input)(x, y);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            input.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            output.set_estimates({
                {0, X},
                {0, Y}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                input.dim(0).set_stride(C);
                input.dim(2).set_stride(1);
                output.dim(0).set_stride(C);
                output.dim(2).set_stride(1);
            }
        }

    private:
        Halide::Var x{"x_ZeroMask"}, y{"y_ZeroMask"};
    };

    /**
     * Compute a toroidal 2D log-chroma histogram for the given 3-channel image.
     * This is an integral part of the Google Fast Fourier Color Constancy algorithm.
     *
     * This method supports float (0-1) images only and requires:
     * - a linear image
     * - a mask for pixels to be ignored
     * - an image, mask, and region that match in x and y dimensions
     */
    Halide::Func toroidal_histogram(const Halide::Func &image, const Halide::Func &external_mask,
                                    const Halide::Region &region) {
        // Reference ffcc/internal/FeatureizeImage.m
        Halide::Var x{"x_toroidal_histogram"}, y{"y_toroidal_histogram"};
        Halide::Expr above_min_intensity{"above_min_intensity_toroidal_histogram"};
        above_min_intensity = histogram_min_intensity < image(x, y, 0) &&
                              histogram_min_intensity < image(x, y, 1) &&
                              histogram_min_intensity < image(x, y, 2);

        Halide::Func mask{"mask_toroidal_histogram"};
        // & required here. && fails checks against reference.
        mask(x, y) = Halide::select(above_min_intensity, 1, 0) & external_mask(x, y);
        // Reference ffcc/internal/Psplat2.m
        Halide::Func u{"u_toroidal_histogram"}, v{"v_toroidal_histogram"};
        Halide::RDom r_image{region, "r_image_toroidal_histogram"};
        u(x, y) = 0.0f;
        u(r_image.x, r_image.y) =
                (log(image(r_image.x, r_image.y, 1)) - log(image(r_image.x, r_image.y, 0))) *
                mask(r_image.x, r_image.y);

        v(x, y) = 0.0f;
        v(r_image.x, r_image.y) =
                (log(image(r_image.x, r_image.y, 1)) - log(image(r_image.x, r_image.y, 2))) *
                mask(r_image.x, r_image.y);

        Halide::Expr e_hist_i{"e_hist_i_toroidal_histogram"}, e_hist_j{"e_hist_j_toroidal_histogram"};
        // Cast to an int because Halide::round produces a float. float % int is not supported.
        e_hist_i = Halide::cast<int>(
                       round((u(r_image.x, r_image.y) - histogram_starting_uv) / histogram_bin_width)
                   ) % histogram_bin_count;
        e_hist_j = Halide::cast<int>(
                       round((v(r_image.x, r_image.y) - histogram_starting_uv) / histogram_bin_width)
                   ) % histogram_bin_count;

        Halide::Var hist_i{"hist_i_toroidal_histogram"}, hist_j{"hist_j_toroidal_histogram"};
        Halide::Func histogram{"histogram_toroidal_histogram"};
        histogram(e_hist_i, e_hist_j) += mask(r_image.x, r_image.y);

        // Reference ffcc/internal/FeatureizeImage.m
        // Normalize histogram
        Halide::Func histogram_sum{"histogram_sum_toroidal_histogram"},
                normalized_histogram("normalized_histogram_toroidal_histogram");;
        Halide::RDom r_hist{
            {{0, histogram_bin_count}, {0, histogram_bin_count}},
            "r_hist_toroidal_histogram"
        };
        histogram_sum() = Halide::sum(histogram(r_hist.x, r_hist.y), "sum_toroidal_histogram");
        normalized_histogram(x, y) = histogram(x, y) / Halide::max(epsilon, histogram_sum());

        return normalized_histogram;
    }

    class ToroidalHistogram : public Generator<ToroidalHistogram> {
    public:
        Input<Buffer<float> > input{"input_ToroidalHistogram", 3};
        Input<Buffer<int> > mask{"mask_ToroidalHistogram", 2};
        Output<Buffer<float> > output{"output_ToroidalHistogram", 2};

        void generate() {
            output(x, y) = toroidal_histogram(
                input,
                mask,
                {{input.dim(0).min(), input.dim(0).extent()}, {input.dim(1).min(), input.dim(1).extent()}}
            )(x, y);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            input.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            mask.set_estimates({
                {0, X},
                {0, Y}
            });

            output.set_estimates({
                {0, histogram_bin_count},
                {0, histogram_bin_count}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                input.dim(0).set_stride(C);
                input.dim(2).set_stride(1);
                output.dim(0).set_stride(C);
                output.dim(2).set_stride(1);
            }
        }

    private:
        Halide::Var x{"x_ToroidalHistogram"}, y{"y_ToroidalHistogram"};
    };

    /**
     * Compute local absolute deviation (LAD) for the given 3-channel image.
     * This is an integral part of the Google Fast Fourier Color Constancy algorithm.
     *
     * LAD is computed over a sliding 3x3 region by taking the average of the absolute
     * difference of perimeter pixels from the centre pixel.
     *
     * This method supports float (0-1) images only and requires:
     * - a mask for pixels to be ignored
     * - an image, mask, and region that match in x and y dimensions
     */
    Halide::Func local_absolute_deviation(const Halide::Func &image, const Halide::Func &mask,
                                          const Halide::Region &bounds) {
        // Reference ffcc/internal/MaskedLocalAbsoluteDeviation.m.
        Halide::Var x{"x_local_absolute_deviation"}, y{"y_local_absolute_deviation"}, c{"c_local_absolute_deviation"};

        // Ensure each element in the image and mask have a perimeter by using repeating edges.
        Halide::Func edge_image = Halide::BoundaryConditions::repeat_edge(image, bounds);
        Halide::Func edge_mask = Halide::BoundaryConditions::repeat_edge(mask, bounds);

        Halide::Func numerator{"numerator_local_absolute_deviation"};
        Halide::RDom r{-1, 3, -1, 3, "r_local_absolute_deviation"}; // 3x3 region

        // Sum absolute differences of perimeter pixels from the centre.
        numerator(x, y, c) += Halide::abs(
            edge_image(x + r.x, y + r.y, c) - edge_image(x, y, c) // Centre pixel excluded via subtraction.
        ) * edge_mask(x + r.x, y + r.y); // Exlude masked pixels in edge-repeated mask

        // Reduce a sum for each centre pixel by counting every unmasked perimeter pixel in region.
        Halide::Func denominator{"denominator_local_absolute_deviation"};
        denominator(x, y) = epsilon; // Prevent division by zero
        denominator(x, y) += Halide::cast<float>(
            edge_mask(x + r.x, y + r.y) // Count unmasked pixels
        ) * !(r.x == 0 && r.y == 0); // Prevent counting the centre pixel

        Halide::Func lad{"lad_local_absolute_deviation"};
        lad(x, y, c) = numerator(x, y, c) / denominator(x, y);

        return lad;
    }

    class LocalAbsoluteDeviation : public Generator<LocalAbsoluteDeviation> {
    public:
        Input<Buffer<float> > input{"input_LocalAbsoluteDeviation", 3};
        Input<Buffer<int> > mask{"mask_LocalAbsoluteDeviation", 2};
        Output<Buffer<float> > output{"output_LocalAbsoluteDeviation", 3};

        Halide::Var x{"x_LocalAbsoluteDeviation"}, y{"y_LocalAbsoluteDeviation"},
                c{"c_LocalAbsoluteDeviation"};

        void generate() {
            output(x, y, c) = local_absolute_deviation(
                input,
                mask,
                {{input.dim(0).min(), input.dim(0).extent()}, {input.dim(1).min(), input.dim(1).extent()}}
            )(x, y, c);
        }

        void schedule_auto() override {
            const int X{x_extent_estimate}, Y{y_extent_estimate}, C{3};

            input.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            mask.set_estimates({
                {0, X},
                {0, Y}
            });

            output.set_estimates({
                {0, X},
                {0, Y},
                {0, C}
            });

            if (layout == Layout::Planar) {
            } else if (layout == Layout::Interleaved) {
                input.dim(0).set_stride(C);
                input.dim(2).set_stride(1);
                output.dim(0).set_stride(C);
                output.dim(2).set_stride(1);
            }
        }
    };
} // namespace photog

HALIDE_REGISTER_GENERATOR(photog::SrgbToLinear, photog_srgb_to_linear);
HALIDE_REGISTER_GENERATOR(photog::RgbToLinear, photog_rgb_to_linear);
HALIDE_REGISTER_GENERATOR(photog::SrgbToXyz, photog_srgb_to_xyz);
HALIDE_REGISTER_GENERATOR(photog::RgbToXyz, photog_rgb_to_xyz);
HALIDE_REGISTER_GENERATOR(photog::LinearToSrgb, photog_linear_to_srgb);
HALIDE_REGISTER_GENERATOR(photog::LinearToRgb, photog_linear_to_rgb);
HALIDE_REGISTER_GENERATOR(photog::XyzToSrgb, photog_xyz_to_srgb);
HALIDE_REGISTER_GENERATOR(photog::XyzToRgb, photog_xyz_to_rgb);
HALIDE_REGISTER_GENERATOR(photog::Average, photog_average);
HALIDE_REGISTER_GENERATOR(photog::Chromadapt, photog_chromadapt_impl);
HALIDE_REGISTER_GENERATOR(photog::ZeroMask, photog_zero_mask);
HALIDE_REGISTER_GENERATOR(photog::ToroidalHistogram, photog_toroidal_histogram);
HALIDE_REGISTER_GENERATOR(photog::LocalAbsoluteDeviation, photog_local_absolute_deviation);
