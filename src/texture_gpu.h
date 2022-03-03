#ifndef TEXTURE_GPU_H_
#define TEXTURE_GPU_H_

#include <GL/gl3w.h>
#include <cassert>
#include <cstdint>
#include <cstddef>

#include "color.h"
#include "glcorearb.h"

/**
 * Just a wrapper for the upload of a raw texture to the GPU
 **/

#include <iostream>
struct sGPU_Texture {
    uint32_t  gpu_id = 0;

    uint32_t  width = 0;
    uint32_t  heigth = 0;

    void upload_raw_data(const void* raw_data,
                         const uint32_t data_width,
                         const uint32_t data_heigth,
                         const GLenum format,
                         const GLenum data_type) {
        assert(raw_data != NULL && "Cannot upload empty texture to GPU");

        // Recrete texture in the case that the shape has changed
        if (data_width != width || data_heigth != heigth) {
            glDeleteTextures(1,
                             &gpu_id);
            glGenTextures(1,
                          &gpu_id);

            width = data_width;
            heigth = data_heigth;
        }

        glBindTexture(GL_TEXTURE_2D, gpu_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     format,
                     width,
                     heigth,
                     0,
                     GL_RGBA,
                     data_type,
                     raw_data);

        glGenerateMipmap(GL_TEXTURE_2D);


        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, gpu_id);
    }
    void unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif // TEXTURE_GPU_H_
