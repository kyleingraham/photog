#ifndef PHOTOG_COLOR_H
#define PHOTOG_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

enum PhotogWorkingSpace {
    Srgb
};

enum PhotogChromadaptMethod {
    Bradford
};

/** Standard illuminants of various vintages.
 *
 * References:
 *  http://www.brucelindbloom.com/Eqn_ChromAdapt.html
 *  https://en.wikipedia.org/wiki/Standard_illuminant
 *  https://www.image-engineering.de/library/technotes/753-cie-standard-illuminants
 *  https://www.xrite.com/service-support/understanding_illuminants
 *  https://www.xrite.com/service-support/what_illuminants_are_available_in_xrite_hardware_and_software
 */
enum PhotogIlluminant {
    /** Tungsten-filament (incandescent) lighting */
    A,      // 2856K
    /** Deprecated daylight representations based on A */
    B,      // direct @ 4874K
    C,      // shade @ 6774K
    /** Favoured daylight representations */
    D50,    // 5003K
    D55,    // ~5500K
    D65,    // 6504K
    D75,    // ~7500K
    /** Hypothetical equal-energy radiator */
    E,      // approximated by D illuminant @ 5455K
    /** Fluorescent lighting */
    F2,     // semi-broadband "cool-white" @ 4230K
    F7,     // broadband @ 6500K
    F11     // tri-band @ 4000K
};

/** Chromatically adapt RGB input from the given source illuminant to the given
 * destination illuminant.
 *
 * We employ the von Kries coefficient law for chromatic adaptation as we do
 * in @ref photog_chromadapt "photog_chromadapt".
 *
 * Here both the source and destination tristimulus values must be supplied by
 * the user hence the "_diy" prefix.
 *
 * @param input pointer to float array containing an RGB image to be adapted.
 * Pixel values should be between 0 and 1.
 *
 * @param width width (in pixels) of the input image.
 *
 * @param height height (in pixels) of the input image.
 *
 * @param source_tristimulus pointer to float array containing an XYZ estimate
 * tristimulus for the source illuminant.
 *
 * @param working_space working space of the input image (see
 * @ref PhotogWorkingSpace "working spaces"). Ensures that color space
 * conversions are accurate.
 *
 * @param chromadapt_method method by which input is chromatically-adapted (see
 * @ref PhotogChromadaptMethod "chromatic adaptation methods").
 *
 * @param dest_tristimulus pointer to float array containing an XYZ tristimulus
 * for the destination illuminant.
 *
 * @param output pointer to float array that will receive the chromatically-
 * adapted RGB image. This array must be equal in size to the input array.
 * Pixel values will be between 0 and 1.
 */
void photog_chromadapt_diy(float *input, int width, int height,
                           float *source_tristimulus,
                           PhotogWorkingSpace working_space,
                           PhotogChromadaptMethod chromadapt_method,
                           float *dest_tristimulus, float *output);

/** Chromatically adapt RGB input from the estimated source illuminant of the
 * input image to the given destination illuminant.
 *
 * The source illuminant is estimated using the gray-world method.
 *
 * We employ the von Kries coefficient law for chromatic adaptation. A cone
 * response under a source illuminant is converted to one under a destination
 * illuminant via diagonal scaling of the cone response components. The output
 * of the chromatic adaptation between a pair of illuminants can be tweaked by
 * chromatic adaption methods which define the conversion to and from XYZ to
 * cone responses (LMS).
 *
 * References:
 *  http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html
 *  https://en.wikipedia.org/wiki/Von_Kries_coefficient_law#Chromatic_adaptation
 *  https://en.wikipedia.org/wiki/CIE_1931_color_space#Color_matching_functions
 *  https://en.wikipedia.org/wiki/LMS_color_space
 *  http://scottburns.us/chromatic-adaptation-transform-by-reflectance-reconstruction/
 *
 * @param input pointer to float array containing an RGB image to be adapted.
 * Pixel values should be between 0 and 1.
 *
 * @param width width (in pixels) of the input image.
 *
 * @param height height (in pixels) of the input image.
 *
 * @param working_space working space of the input image (see
 * @ref PhotogWorkingSpace "working spaces"). Ensures that color space
 * conversions are accurate.
 *
 * @param chromadapt_method method by which input is chromatically-adapted (see
 * @ref PhotogChromadaptMethod "chromatic adaptation methods").
 *
 * @param dest_illuminant destination illuminant for chromatic adaptation (see
 * @ref PhotogIlluminant "illuminants").
 *
 * @param output pointer to float array that will receive the chromatically-
 * adapted RGB image. This array must be equal in size to the input array.
 * Pixel values will be between 0 and 1.
 */
void photog_chromadapt(float *input, int width, int height,
                       PhotogWorkingSpace working_space,
                       PhotogChromadaptMethod chromadapt_method,
                       PhotogIlluminant dest_illuminant, float *output);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // PHOTOG_COLOR_H
