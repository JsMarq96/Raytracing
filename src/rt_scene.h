#ifndef RT_SCENE_H_
#define RT_SCENE_H_

#include "transform.h"
#include "rt_primitives.h"

#define RT_OBJ_COUNT 50

struct sRT_Scene {
    bool       is_obj_enabled[RT_OBJ_COUNT] = {};
    eRT_Prim   obj_primitive[RT_OBJ_COUNT] {};
    sTransform transforms[RT_OBJ_COUNT] = {};

};


#endif // RT_SCENE_H_
