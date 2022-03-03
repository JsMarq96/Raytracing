#ifndef RT_SCENE_H_
#define RT_SCENE_H_

#include "color.h"
#include "glcorearb.h"
#include "math.h"
#include "render_context.h"
#include "texture.h"
#include "transform.h"
#include "rt_primitives.h"
#include "camera.h"
#include "texture_gpu.h"

#include <cstdint>

#define RT_OBJ_COUNT 50

struct sRT_Scene {
    // Object data
    bool       is_obj_enabled[RT_OBJ_COUNT] = {};
    eRT_Prim   obj_primitive[RT_OBJ_COUNT] {};
    sTransform obj_transforms[RT_OBJ_COUNT] = {};
    uColor_RGBA8   obj_color[RT_OBJ_COUNT] = {};

    //
    uint32_t render_width = 0;
    uint32_t render_heigth = 0;

    void init() {
        memset(is_obj_enabled, false, sizeof(sRT_Scene::is_obj_enabled));
        memset(obj_color, 0, sizeof(sRT_Scene::obj_color));
    }

    // Helper function
    inline uint32_t get_fb_index(const uint32_t x, const uint32_t y) const {
        return x + (y * render_width);
    }

    void render_to_texture(const sCamera &camera,
                           const float FOV,
                           sGPU_Texture *framebuffer) const {
        sVector3 ray_origin = camera.position;

        float tan_half_FOV = tan(to_radians(FOV) / 2.0f);
        float aspect_ratio = render_width / (float) render_heigth;

        uColor_RGBA8 *raw_fbuffer = (uColor_RGBA8*) malloc(sizeof(uColor_RGBA8) * render_heigth * render_width);
        memset(raw_fbuffer, 0, sizeof(uColor_RGBA8) * render_heigth * render_width);

        for (uint32_t x = 0; x < render_width; x++) {
            for (uint32_t y = 0; y < render_heigth; y++) {
                // Compute the ray dir:
                float u = (x / (float) render_width) - 0.5f;
                float v = (y / (float) render_heigth) - 0.5f;
                // (u * cam.right) + (v * cam.up) + cam.pos + cam.forward
                sVector3 image_coords = camera.s.mult(u).sum(camera.u.mult(v)).sum(camera.position).sum(camera.f);
                sVector3 ray_dir = image_coords.subs(ray_origin);

                // Raytracing
                // TODO: extract more info from the collision: point, depht...
                uColor_RGBA8 out_color = {0, 0, 255, 1};
                float out_depth = -1000.0f;
                for(uint16_t i = 0; i <  RT_OBJ_COUNT; i++) {
                    if (!is_obj_enabled[i]) {
                        continue;
                    }

                    float obj_depth = -1000.0f;
                    switch(obj_primitive[i]) {
                        case RT_SPHERE: obj_depth = ray_sphere_collision(ray_origin,
                                                                         ray_dir,
                                                                         obj_transforms[i]); break;
                        case RT_PLANE: break;
                        case RT_PRIMITIVE_COUNT: break; // For the warning
                    }

                    if (obj_depth > out_depth) {
                        out_depth = obj_depth;
                        out_color = obj_color[i];
                    }
                }

                // Store color on texture
                raw_fbuffer[get_fb_index(x, y)] = out_color;
            }
        }
        // Upload raw data to GPU texture
        framebuffer->upload_raw_data((void*) raw_fbuffer,
                                     render_width,
                                     render_heigth,
                                     GL_RGBA8,
                                     GL_UNSIGNED_BYTE);

        free(raw_fbuffer);
    }
};


#endif // RT_SCENE_H_
