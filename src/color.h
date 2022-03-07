#ifndef COLOR_H_
#define COLOR_H_

#include <cstdint>
#include "math.h"

/**
 * Color format definitions
 * */

union uColor_RGBA8 {
    struct {
        uint8_t R = 0;
        uint8_t G = 0;
        uint8_t B = 0;
        uint8_t A = 0;
    };

    uint32_t  color;
};

inline uColor_RGBA8 LERP(const uColor_RGBA8 &c1,
                         const uColor_RGBA8 &c2,
                         const float t) {
    return {
    (uint8_t) LERP(c1.R, c2.R, t),
    (uint8_t) LERP(c1.G, c2.G, t),
    (uint8_t) LERP(c1.B, c2.B, t),
    (uint8_t) LERP(c1.A, c2.A, t) };
}

#endif // COLOR_H_
