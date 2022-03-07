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

// ===================
// PRIMITIVE NORMALS
// ===================


#endif // RT_PRIMITIVES_H_
