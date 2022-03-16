#ifndef RT_SCENE_H_
#define RT_SCENE_H_

#include "color.h"
#include "glcorearb.h"
#include "math.h"
#include "render_context.h"
#include "rt_shading.h"
#include "texture.h"
#include "transform.h"
#include "rt_primitives.h"
#include "camera.h"
#include "texture_gpu.h"
#include "vector.h"
#include "rt_material.h"

#include <cstdint>

#define RT_OBJ_COUNT 50

struct sRT_Scene {
    // Object data
    bool       is_obj_enabled[RT_OBJ_COUNT] = {};
    eRT_Prim   obj_primitive[RT_OBJ_COUNT] {};
    sRT_Material obj_material[RT_OBJ_COUNT] = {};
    sTransform obj_transforms[RT_OBJ_COUNT] = {};

    // Light data
    sVector3  light_position = {};
    uColor_RGBA8 light_color = {};
    uint32_t render_width = 0;
    uint32_t render_heigth = 0;

    void init() {
        memset(is_obj_enabled, false, sizeof(sRT_Scene::is_obj_enabled));
        memset(obj_material, 0, sizeof(sRT_Scene::obj_material));
    }

    // Helper function
    inline uint32_t get_fb_index(const uint32_t x, const uint32_t y) const {
        return x + (y * render_width);
    }

    void render_to_texture(const sCamera &camera,
                           const float FOV,
                           sGPU_Texture *framebuffer) const {
        float tan_half_FOV = tan(to_radians(FOV / 2.0f));
        float aspect_ratio = render_width / (float) render_heigth;

        sVector3 ray_origin = camera.position;

        uColor_RGBA8 *raw_fbuffer = (uColor_RGBA8*) malloc(sizeof(uColor_RGBA8) * render_heigth * render_width);
        memset(raw_fbuffer, 0, sizeof(uColor_RGBA8) * render_heigth * render_width);


        sMat44 inv_view_mat = {};
        camera.view_mat.invert(&inv_view_mat);

        ray_origin = inv_view_mat.multiply(sVector3{0.0f, 0.0f, 0.0f});
        ray_origin = camera.position;

         for (uint32_t x = 0; x < render_width; x++) {
            for (uint32_t y = 0; y < render_heigth; y++) {
                // Copmute NDC coords
                float u = ((float) x / render_width) - 0.5f;
                float v = ((float) y / render_heigth) - 0.5f;
                u *= aspect_ratio * tan_half_FOV;
                v *= tan_half_FOV;

                // Compute the viewpoint of the NDC
                sVector3 ray_dir = {u, v, -1.0f};

                // Transform to world coordinates
                ray_dir = inv_view_mat.multiply(ray_dir);
                // Compute the direction
                ray_dir = ray_dir.subs(ray_origin).normalize();


                sVector3 col_point = {};
                uint16_t col_obj_id = 0;

                uColor_RGBA8 out_color = {};
                if (raycast(ray_origin,
                            ray_dir,
                            &col_point,
                            &col_obj_id)) {
                    // There is an object, so we test if its in teh shadow, and we render it
                    // Add basic diffuse color
                    sVector3 normal = sphere_normal(col_point, obj_transforms[col_obj_id]);

                    // Shadows
                    sVector3 shadow_ray_origin = col_point.sum(normal.mult(0.0001f));
                    sVector3 shadow_ray_dir = light_position.subs(shadow_ray_origin).normalize();
                    sVector3 shadow_col_point = {};
                    uint16_t shadow_col_id = 0;

                    // Compute color
                    out_color = simple_shader(col_point,
                                              normal,
                                              obj_material[col_obj_id],
                                              light_position,
                                              light_color,
                                              camera.position,
                                              raycast(shadow_ray_origin,
                                                      shadow_ray_dir,
                                                      &shadow_col_point,
                                                      &shadow_col_id));

                    //normal = normal.sum({1.0f, 1.0f, 1.0f}).mult(0.5f).normalize();
                    //out_color = uColor_RGBA8{(uint8_t) (normal.x * 255.0f), (uint8_t) (normal.y * 255.0f), (uint8_t) (normal.z * 255.0f), 255};
                } else {
                    out_color = simple_sky_shader(ray_dir.normalize());
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

    inline bool raycast(const sVector3 &ray_origin,
                        const sVector3 &ray_direction,
                        sVector3 *collision_point,
                        uint16_t *object_id) const {
        uint16_t it_col_object = 0;
        float it_t = 1000.0f;
        for(uint16_t i = 0; i < RT_OBJ_COUNT; i++) {
            if (!is_obj_enabled[i]) {
                continue;
            }

            float t_of_hit = -1.0f;
            // Test if the ray hits the object
            switch (obj_primitive[i]) {
                case RT_SPHERE: t_of_hit = ray_sphere_collision(ray_origin,
                                                                ray_direction,
                                                                obj_transforms[i]); break;
                case RT_PLANE: t_of_hit = ray_plane_collision(ray_origin,
                                                              ray_direction,
                                                              obj_transforms[i].position,
                                                              sVector3{}); break;
                case RT_PRIMITIVE_COUNT: break; // Just for removing the warning
            }

            if (t_of_hit > 0.0f) {
                // A hit has happend
                if (it_t > t_of_hit) {
                    // The hit is closer, so we keep it
                    it_t = t_of_hit;
                    it_col_object = i;
                }
            }
        }

        if (it_t < 1000.0f) {
            // A hit has happend
            *object_id = it_col_object;
            // Calculate the position that the ray would have at the time of the
            // collision
            *collision_point = ray_direction.normalize().mult(it_t).sum(ray_origin);
            return true;
        }
        return false;
    }
};


#endif // RT_SCENE_H_
