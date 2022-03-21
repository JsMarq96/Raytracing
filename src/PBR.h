#ifndef PBR_H_
#define PBR_H_

#include "color.h"
#include "math.h"
#include "vector.h"

/**
 * PBR functions
 * */

namespace PBR {
    inline sVector3 schlick_fresnel(const float cos_theta,
                                       const sVector3 &F0) {
       float power = pow(1.0f - cos_theta, 5.0f);
       // Need normalization
       return F0.sum(sVector3{1.0f, 1.0f, 1.0f}.subs(F0)).mult(power);
   }

    // Epic Games's aproximation GGX
   inline float geometry_atenuation(const float roughness,
                                   const float n_dot_l,
                                   const float n_dot_v) {
       // Using Epic game's aproximation
       float r = roughness + 1.0f;
       float k = (r * r) / 8.0f;
       float G1 = n_dot_v / (n_dot_v * (1.0f - k) + k);
       float G2 = n_dot_l / (n_dot_l * (1.0f - k) + k);

       return G1 * G2;
   }

    // Schlick GGX aproximation using Smith's method
   inline float normal_distribution(const float roughness,
                                    const float n_dot_h) {
       float alpha = roughness * roughness;
       float squared_alpha = alpha * alpha;

       float denom = ((n_dot_h * n_dot_h) * (squared_alpha - 1.0f) ) + 1.0f;
       return squared_alpha / (PI * denom * denom);
   }
}



#endif // PBR_H_
