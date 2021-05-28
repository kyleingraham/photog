#ifndef PHOTOG_COLOR_H
#define PHOTOG_COLOR_H

#include "HalideBuffer.h"

struct halide_buffer_t;

// TODO: Put all headers in 'include' directories
#ifdef __cplusplus
extern "C" {
#endif

enum PhotogWorkingSpace {
    srgb
};

halide_buffer_t* photog_get_rgb_to_xyz_xfmr(PhotogWorkingSpace working_space);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif //PHOTOG_COLOR_H
