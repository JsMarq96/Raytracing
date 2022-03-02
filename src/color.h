#ifndef COLOR_H_
#define COLOR_H_

#include <cstdint>

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

#endif // COLOR_H_
