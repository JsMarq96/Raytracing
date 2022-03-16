#ifndef RT_MATERIAL_H_
#define RT_MATERIAL_H_

#include "color.h"
#include "math.h"

struct sRT_Material {
    uColor_RGBA8 albedo = {0,0,0,0};
    float roughness = 0.0f;
    float metalness = 0.0f;
};

#endif // RT_MATERIAL_H_
