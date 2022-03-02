#ifndef RT_SCENE_H_
#define RT_SCENE_H_

#include "color.h"
#include "math.h"
#include "render_context.h"
#include "texture.h"
#include "transform.h"
#include "rt_primitives.h"
#include "camera.h"
#include <cstdint>

#define RT_OBJ_COUNT 50

struct sRT_Scene {
    // Object data
    bool       is_obj_enabled[RT_OBJ_COUNT] = {};
    eRT_Prim   obj_primitive[RT_OBJ_COUNT] {};
    sTransform obj_transforms[RT_OBJ_COUNT] = {};
    uColor_RGBA8   obj_color[RT_OBJ_COUNT] = {};

    //
    uint16_t render_width = 0;
    uint16_t render_heigth = 0;

    uColor_RGBA8 *framebuffer = NULL;

    void init() {
        memset(is_obj_enabled, false, sizeof(sRT_Scene::is_obj_enabled));

    }

    inline uint16_t get_fb_index(const uint16_t x, const uint16_t y) const {
        return x + (y * render_width);
    }

    void render_to_texture(const sCamera &camera,
                           const float FOV) {
        sVector3 ray_origin = camera.position;

        float tan_half_FOV = tan(to_radians(FOV) / 2.0f);
        float aspect_ratio = render_width / (float) render_heigth;

        sMat44 inv_view_mat;
        camera.view_mat.invert(&inv_view_mat);

        if (framebuffer != NULL) {
            free(framebuffer);
        }
        framebuffer = (uColor_RGBA8*) malloc(sizeof(uColor_RGBA8) * render_heigth * render_width);

        for (uint16_t x = 0; x < render_width; x++) {
            for (uint16_t y = 0; y < render_heigth; y++) {
                // Compute the ray dir:
                float u = (2 * (x + 0.5f) / (float) render_width - 1) * aspect_ratio * camera.zoom;
                float v = (1 - 2 * (y + 0.5) / (float) render_heigth) * camera.zoom;

                sVector3 ray_dir = inv_view_mat.multiply(sVector3{u, v, -1.0f}).normalize();


                // Raytracing
                // TODO: extract more info from the collision: point, depht...
                uColor_RGBA8 out_color = {};
                float out_depth = -1000.0f;
                for(uint16_t i = 0; i <  RT_OBJ_COUNT; i++) {
                    if (!is_obj_enabled[i]) {
                        continue;
                    }

                    float obj_depth = 0.0f;
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
                framebuffer[get_fb_index(x, y)] = out_color;
            }
        }
    }
};


#endif // RT_SCENE_H_
