#ifndef RT_SHADING_H_
#define RT_SHADING_H_

#include "color.h"
#include "math.h"
#include "rt_material.h"
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
                                  const sRT_Material &obj_material,
                                  const sVector3 &light_position,
                                  const uColor_RGBA8 &light_color,
                                  const sVector3 &camera_position,
                                  const bool is_in_shadow) {
    uColor_RGBA8 final_color = obj_material.albedo;
    sVector3 l = light_position.subs(frag_position);
    float light_distance = l.magnitude();
    l = l.normalize();
    float n_dot_l = MAX(dot_prod(l, frag_normal), 0.0f);

    if (is_in_shadow) {
        // Added sort of soft shadows
        final_color = LERP(final_color, {0, 0, 0, 255}, 1.0f - (light_distance / 10.0f));
    }

    // Specular component
    sVector3 r = reflect_vector(l, frag_normal).normalize();
    sVector3 v = frag_position.subs(camera_position).normalize();
    float r_dot_v = MAX(0.0f, dot_prod(r, v));
    float specular = pow(r_dot_v, 32.0f);

    // Add difuse
    //final_color = final_color.multiply_RGB(n_dot_l);

    // Add specular
    final_color = LERP(final_color, light_color, specular);

   // Draw the base color with a bit of shine
    return final_color;
}
#endif // RT_SHADING_H_
