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

enum PhotogIlluminant {
    D65
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
