#ifndef GLTF_PARSER_H_
#define GLTF_PARSER_H_

#include "material.h"
#include "gltf_scene.h"

#include "tiny_gltf.h"

namespace Parser {
    void load_gltf_model(sScene *scene,
                         const char* gltf_root_dir,
                         const bool  is_binary);

    uint32_t* _load_gltf_materials(sScene *scene,
                                   const tinygltf::Model &model);

    uint32_t* _load_gltf_geometry(sScene *scene,
                                  const tinygltf::Model &model,
                                  const uint32_t *mats_gltf_indexing);
};

#endif // GLTF_PARSER_H_
