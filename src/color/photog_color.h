#ifndef PHOTOG_PHOTOG_COLOR_H
#define PHOTOG_PHOTOG_COLOR_H

#include "HalideBuffer.h"

struct halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

enum PhotogWorkingSpace {
    srgb
};

enum ChromadaptMethod {
    bradford
};

halide_buffer_t* photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space);

halide_buffer_t* photog_get_xyz_to_rgb_xfmr(PhotogWorkingSpace working_space);

float photog_get_working_space_gamma(PhotogWorkingSpace workingSpace);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif //PHOTOG_PHOTOG_COLOR_H
