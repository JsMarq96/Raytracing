#ifndef RT_SHADING_H_
#define RT_SHADING_H_

#include "color.h"
#include "math.h"
#include "PBR.h"
#include "rt_material.h"
#include "vector.h"
#include <type_traits>

inline uColor_RGBA8 simple_sky_shader(const sVector3 &norm_ray_dir) {
    uColor_RGBA8 sky_color_1 = {255, 255, 255, 255};
    uColor_RGBA8 sky_color_2 = {0, 20, 150, 255};

    // Compute the background gradient
    float t = 0.5f * (norm_ray_dir.normalize().y + 1.0f);
    return LERP(sky_color_1, sky_color_2, t);
}

inline uColor_RGBA8 GI_shader(const sVector3 &frag_position,
                                  const sVector3 &frag_normal,
                                  const sRT_Material &obj_material,
                                  const sVector3 &light_position,
                                  const uColor_RGBA8 &light_color,
                                  const sVector3 &camera_position,
                              const float fragment_shadow_distance) {
    //early out
    if (fragment_shadow_distance < 0.0f) {
        //return {0, 0, 0, 255};
    }

    sVector3 final_color = color_to_vector(obj_material.albedo);
    sVector3 l = light_position.subs(frag_position);
    float light_distance = l.magnitude();//l.x * l.x + l.y * l.y + l.z * l.z;
    l = l.normalize();
    float n_dot_l = MAX(dot_prod(l, frag_normal), 0.0f);

    // Specular component
    sVector3 r = reflect_vector(l, frag_normal).normalize();
    sVector3 v = frag_position.subs(camera_position).normalize();
    float r_dot_v = MAX(0.0f, dot_prod(r, v));
    float specular = pow(r_dot_v, 32.0f) * obj_material.roughness;

    // Add difuse
    final_color = final_color.mult(n_dot_l);
    final_color = final_color.mult(1.0f - specular).sum(color_to_vector(light_color).mult(specular));
    //std::cout << specular << std::endl;
    // Add specular
    //final_color = LERP_3D(final_color, color_to_vector(light_color), specular);

    //uColor_RGBA8 light_intensity = uColor_RGBA8{255, 255, 255, 255};

    //final_color = final_color.mult(((light_distance * light_distance)));
    //std::cout << light_distance << std::endl;

// Draw the base color with a bit of shine
    return vector_to_color(final_color);

}

inline uColor_RGBA8 simple_shader(const sVector3 &frag_position,
                                  const sVector3 &frag_normal,
                                  const sRT_Material &obj_material,
                                  const sVector3 &light_position,
                                  const uColor_RGBA8 &light_color,
                                  const sVector3 &camera_position,
                                  const float fragment_shadow_distance) {
    //early out
    if (fragment_shadow_distance < 0.0f) {
        return {0, 0, 0, 255};
    }

    uColor_RGBA8 final_color = obj_material.albedo;
    sVector3 l = light_position.subs(frag_position);
    float light_distance = l.x * l.x + l.y * l.y + l.z * l.z;
    l = l.normalize();
    float n_dot_l = MAX(dot_prod(l, frag_normal), 0.0f);

    // Specular component
    sVector3 r = reflect_vector(l, frag_normal).normalize();
    sVector3 v = frag_position.subs(camera_position).normalize();
    float r_dot_v = MAX(0.0f, dot_prod(r, v));
    float specular = pow(r_dot_v, 32.0f) * obj_material.roughness;

    // Add difuse
    final_color = final_color.multiply_RGB(n_dot_l);

    // Add specular
    final_color = LERP(final_color, light_color, specular);

    //uColor_RGBA8 light_intensity = uColor_RGBA8{255, 255, 255, 255};

    //final_color = final_color.multiply_RGB(1.0f / (light_distance * light_distance));
    //std::cout << light_distance << std::endl;

// Draw the base color with a bit of shine
    return final_color;
}

inline uColor_RGBA8 PBR_shader(const sVector3 &frag_position,
                               const sVector3 &frag_normal,
                               const sRT_Material &obj_material,
                               const sVector3 &light_position,
                               const uColor_RGBA8 &light_color,
                               const sVector3 &camera_position,
                               const float fragment_shadow_distance) {
    //early out
    if (fragment_shadow_distance < 0.0f) {
        return {0, 0, 0, 255};
    }

    // F0 aproximation
    sVector3 F0 = {0.04f, 0.04f, 0.04f};
    F0 = LERP_3D(F0, color_to_vector(obj_material.albedo), obj_material.metalness);

    sVector3 l = light_position.subs(frag_position).normalize();
    sVector3 v = frag_position.subs(camera_position).normalize();
    sVector3 h = v.sum(frag_normal).normalize();
    float n_dot_l = MAX(dot_prod(l, frag_normal), 0.0f);
    float n_dot_h = MAX(dot_prod(frag_normal, h), 0.0f);
    float n_dot_v = MAX(dot_prod(frag_normal, v), 0.0f);
    float h_dot_v = MAX(dot_prod(h, v), 0.0f);

    // Direct lighting
    // Specular GGX =====
    sVector3 fresnel = PBR::schlick_fresnel(h_dot_v, F0);

    float G = PBR::geometry_atenuation(obj_material.roughness,
                                               n_dot_l,
                                               n_dot_v);
    float NDF = PBR::normal_distribution(obj_material.roughness,
                                         n_dot_h);

    float denominator = (4.0f * n_dot_v * n_dot_l) + 0.0001f;

    sVector3 kS = fresnel;

    // specular = (F * G * NDF) / ((4.0 * n_dot_l * n_dot_v) + 0.00001)
    sVector3 specular_contribution = fresnel.mult(NDF * G).mult(1.0f/denominator);

    // Diffuse lambert =======
    // Diffuse scattering: metal object does not absorve any, so there is no scattering
    sVector3 kD = sVector3{1.0f, 1.0f, 1.0f}.subs(kS).mult( obj_material.metalness);

    sVector3 diffuse_contribution = color_to_vector(obj_material.albedo).mult(kD);

    return vector_to_color(specular_contribution.sum(diffuse_contribution).normalize().mult(n_dot_l));
}
#endif // RT_SHADING_H_
