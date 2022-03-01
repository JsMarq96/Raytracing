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
    sVector3 oc = ray_origin.subs(sphere_transform.position);
    float b = dot_prod(oc, ray_dir);
    float c = dot_prod(oc, oc) - (radius * radius);
    float h = (b * b) - c;
    if (h < 0.0f) {
        return -1.0f;
    }

    return -b - sqrt(h);
}

// ===================
// PRIMITIVE NORMALS
// ===================


#endif // RT_PRIMITIVES_H_
