#ifndef TEXTURE_GPU_H_
#define TEXTURE_GPU_H_

#include <GL/gl3w.h>
#include <cassert>
#include <cstdint>

#include "color.h"
#include "glcorearb.h"


struct sGPU_Texture {
    uint32_t  gpu_id = 0;

    uint16_t  width = 0;
    uint16_t  heigth = 0;

    void upload_raw_data(const void* raw_data,
                         const uint16_t data_width,
                         const uint16_t data_heigth,
                         const GLenum format) {
        if (data_width != width || data_heigth != heigth) {

        }
    }

    void bind() const;
    void unbind() const;
};

#endif // TEXTURE_GPU_H_
