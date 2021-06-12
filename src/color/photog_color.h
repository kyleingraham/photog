#ifndef PHOTOG_PHOTOG_COLOR_H
#define PHOTOG_PHOTOG_COLOR_H

// Forward declare because we would like to not need Halide to work with
// this library when compiled.
struct halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

enum PhotogWorkingSpace {
    Srgb
};

enum ChromadaptMethod {
    Bradford
};

enum Illuminant {
    D65
};

halide_buffer_t *photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space);

halide_buffer_t *photog_get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space);

float photog_get_gamma(PhotogWorkingSpace workingSpace);

halide_buffer_t *photog_get_tristimulus(Illuminant illuminant);

halide_buffer_t *photog_create_tristimulus(float *tristimulus);

halide_buffer_t *photog_get_xyz_to_lms_xfmr(ChromadaptMethod chromadapt_method);

halide_buffer_t *photog_get_lms_to_xyz_xfmr(ChromadaptMethod chromadapt_method);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif //PHOTOG_PHOTOG_COLOR_H
