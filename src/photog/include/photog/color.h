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

void photog_chromadapt_diy_p3(float *input, int width, int height,
                              float *source_tristimulus,
                              PhotogWorkingSpace working_space,
                              PhotogChromadaptMethod chromadapt_method,
                              float *dest_tristimulus, float *output);

void photog_chromadapt_p3(float *input, int width, int height,
                          PhotogWorkingSpace working_space,
                          PhotogChromadaptMethod chromadapt_method,
                          PhotogIlluminant dest_illuminant, float *output);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // PHOTOG_COLOR_H
