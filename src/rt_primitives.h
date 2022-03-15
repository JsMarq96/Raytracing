#ifndef RT_PRIMITIVES_H_
#define RT_PRIMITIVES_H_

#include "math.h"
#include "transform.h"
#include "vector.h"
#include <cstdint>

enum eRT_Prim : uint8_t {
    RT_SPHERE = 0,
    RT_PLANE,
    RT_PRIMITIVE_COUNT
};



// ===================
// RAY-COLLISIONS
// ===================

// ? is this okay?
inline float ray_sphere_collision(const sVector3 &ray_origin,
                                 const sVector3 &ray_dir,
                                 const sTransform &sphere_transform) {
    float radius = sphere_transform.scale.x;
    sVector3 L = ray_origin.subs(sphere_transform.position);
    float a  = dot_prod(ray_dir, ray_dir);
    float b = 2.0f * dot_prod(ray_dir, L);
    float c = dot_prod(L, L) - (radius * radius);

    float res_1 = -1.0f, res_2 = -1.0f;

    if (solve_quadratic_equation(a, b, c, &res_1, &res_2)) {
        // We only care (FOR NOW) of the contact point time, and that
        // is the firsdt result of the equation
        return res_1;
    }

    return -1.0f;
}


inline float ray_plane_collision(const sVector3 &ray_origin,
                                 const sVector3 &ray_dir,
                                 const sVector3 &plane_origin,
                                 const sVector3 &plane_normal) {
    float dot_ray_plane = dot_prod(ray_dir, plane_normal);

    if (dot_ray_plane > 0.0f) {
        float t = -dot_prod(plane_origin.subs(ray_origin), plane_normal) / dot_ray_plane;

        if (t >= 0.0) {
            return t;
        }
    }

    return -1.0f;
}
// ===================
// PRIMITIVE NORMALS
// ===================
inline sVector3 sphere_normal(const sVector3 &point, const sTransform &sphere_transform) {
    return sphere_transform.position.subs(point).invert().normalize();
}

inline sVector3 plane_normal(const sVector3 &point,
                             const sVector3 &plane_origin,
                             const sVector3 &plane_normal) {
    return plane_normal;
}

#endif // RT_PRIMITIVES_H_
