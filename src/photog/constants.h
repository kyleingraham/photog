#ifndef PHOTOG_CONSTANTS_H
#define PHOTOG_CONSTANTS_H

namespace photog {
    enum class Layout {
        Planar, Interleaved
    };

    constexpr float histogram_bin_width = 1.0f / 32;

    constexpr float histogram_min_intensity = 1.0f / 256; // This must be greater than zero.

    constexpr uint8_t histogram_bin_count = 64;

    constexpr float histogram_starting_uv = -0.3125;
}

#endif // PHOTOG_CONSTANTS_H
