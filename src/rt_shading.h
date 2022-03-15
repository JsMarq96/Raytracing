#ifndef RT_SHADING_H_
#define RT_SHADING_H_

#include "color.h"
#include "math.h"
#include "rt_scene.h"
#include "vector.h"

inline uColor_RGBA8 simple_sky_shader(const sVector3 &norm_ray_dir) {
    uColor_RGBA8 sky_color_1 = {255, 255, 255, 255};
    uColor_RGBA8 sky_color_2 = {0, 0, 0, 255};

    // Compute the background gradient
    float t = 0.5f * (norm_ray_dir.normalize().y + 1.0f);
    return LERP(sky_color_1, sky_color_2, t);
}

inline uColor_RGBA8 simple_shader(const sVector3 &frag_position,
                                  const sVector3 &frag_normal,
                                  const uColor_RGBA8 &obj_color,
                                  const sVector3 &light_position,
                                  const uColor_RGBA8 &light_color,
                                  const bool is_in_shadow) {
    if (is_in_shadow) {
        return LERP(obj_color, {0, 0, 0, 255}, 0.5f);
    }

   // Draw the base color with a bit of shine
   sVector3 l = light_position.subs(frag_position).normalize();
   float n_dot_l = MAX(dot_prod(l, frag_normal), 0.0f);

   return LERP(obj_color, light_color, n_dot_l);
}
#endif // RT_SHADING_H_
