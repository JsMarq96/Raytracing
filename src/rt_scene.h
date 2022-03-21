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

                // Store color on texture
                raw_fbuffer[get_fb_index(x, y)] = get_ray_color(ray_origin,
                                                                ray_dir,
                                                                camera.position,
                                                                1);
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

    inline uColor_RGBA8 get_ray_color(const sVector3 &ray_origin,
                                      const sVector3 &ray_dir,
                                      const sVector3 &camera_position,
                                      const uint16_t allowed_bounces) const {
        sVector3 col_point = {};
        uint16_t col_obj_id = 0;

        uint16_t *obj_bounce_trayectory = (uint16_t*) malloc(sizeof(uint16_t) * (allowed_bounces + 1));
        // Note: maybe we can also mix this two allocs
        sVector3 *obj_bounce_point = (sVector3*) malloc(sizeof(sVector3) * (allowed_bounces + 1));
        sVector3 *bounce_point_normal = (sVector3*) malloc(sizeof(sVector3) * (allowed_bounces + 1));
        uint16_t bounce_it = 0;

        obj_bounce_point[0] = ray_origin;
        bounce_point_normal[0] = ray_dir;

        for(; bounce_it < allowed_bounces; bounce_it++) {
            sVector3 normal = {};
            if (raycast(obj_bounce_point[bounce_it],
                        bounce_point_normal[bounce_it],
                        &col_point,
                        &col_obj_id)) {
                //std::cout << "en" << std::endl;
                normal = sphere_normal(col_point,
                                       obj_transforms[col_obj_id]);

                // Prepare for the next ray iteration
                obj_bounce_point[bounce_it+1] = col_point;
                obj_bounce_trayectory[bounce_it+1] = col_obj_id;
                bounce_point_normal[bounce_it+1] = reflect_vector(bounce_point_normal[bounce_it],
                                                                  normal);
            } else {
                // Early stop if there is no figure to bounce off
                //out_color = simple_sky_shader(ray_dir.normalize());
                break;
            }
        }
        //std::cout << bounce_it << std::endl;

        // Start iterations with sky color (if any)
        uColor_RGBA8 out_color = simple_sky_shader(bounce_point_normal[bounce_it].normalize());

        // traceback the bounces
        for(uint8_t i = bounce_it; i > 0; i--) {
            //std::cout << i << std::endl;
            // Shadows
            sVector3 shadow_ray_origin = col_point.sum(bounce_point_normal[i].mult(0.01f));
            sVector3 shadow_ray_dir = light_position.subs(shadow_ray_origin).normalize();
            sVector3 shadow_col_point = {};
            uint16_t shadow_col_id = 0;

            float fragment_shadow_distance = -1.0f;
            if (!raycast(shadow_ray_origin,
                        shadow_ray_dir,
                        &shadow_col_point,
                        &shadow_col_id)) {
                fragment_shadow_distance = light_position.subs(shadow_col_point).magnitude();
             }

            // Compute color
            uColor_RGBA8 frag_color = GI_shader(obj_bounce_point[i],
                                                 bounce_point_normal[i],
                                                 obj_material[obj_bounce_trayectory[i]],
                                                 light_position,
                                                 light_color,
                                                 camera_position,
                                                 fragment_shadow_distance);

            // Mix those colors based on the metalness
            //out_color = out_color.sum_RGB(frag_color);
            out_color = frag_color;
            //out_color = LERP(frag_color, out_color, 1.0f - obj_material[obj_bounce_trayectory[i]].metalness);
        }

        free(obj_bounce_point);
        free(obj_bounce_trayectory);
        free(bounce_point_normal);

        return out_color;
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
