#include "../gltf_parser.h"
#include <cstddef>
#include <cstring>
#include <type_traits>
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

void Parser::load_gltf_model(      sScene  *scene,
                             const char*   gltf_root_dir,
                             const bool is_binary) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string error, warn;

    bool parse_result;

    if (is_binary) {
        parse_result = loader.LoadBinaryFromFile(&model,
                                                 &error,
                                                 &warn,
                                                 gltf_root_dir);

    } else {
        parse_result = loader.LoadASCIIFromFile(&model,
                                                 &error,
                                                 &warn,
                                                 gltf_root_dir);
    }

    assert(parse_result && "Error parsing GLTF model");

    uint32_t *gltf_material_indexing = Parser::_load_gltf_materials(scene,
                                                                    model);
    uint32_t *gltf_mesh_indexing = Parser::_load_gltf_geometry(scene,
                                                               model,
                                                               gltf_material_indexing);

    // Load nodes
    for(size_t node_i = 0; node_i <  model.nodes.size(); node_i++) {
        uint16_t scene_node_index = 0;
        for(; scene_node_index < MAX_NODE_COUNT; scene_node_index++) {
            if (!scene->node_is_full[scene_node_index]) {
                break;
            }
        }
        tinygltf::Node *tiny_node = &model.nodes[node_i];

        std::cout << "Add node at" << scene_node_index << std::endl;
        scene->node_name_index_storage.add(tiny_node->name.c_str(),
                                           tiny_node->name.size()+1,
                                           scene_node_index);

        scene->enabled[scene_node_index] = true;
        scene->node_is_full[scene_node_index] = true;
        scene->mesh_of_object[scene_node_index] = gltf_mesh_indexing[tiny_node->mesh];

        scene->models[scene_node_index].set_identity();

        if (tiny_node->translation.size() >= 3) {
            sVector3 position = { (float) tiny_node->translation[0],
                                  (float) tiny_node->translation[1],
                                  (float) tiny_node->translation[2] };

            scene->models[scene_node_index].set_position(position);
        }

        if (tiny_node->scale.size() >= 3) {
            sVector3 scale = { (float)tiny_node->scale[0],
                               (float) tiny_node->scale[1],
                               (float) tiny_node->scale[2] };
            scene->models[scene_node_index].set_scale(scale);
            std::cout << "scale" << std::endl;
        }

        // Store the node name
        strcpy(scene->node_name[scene_node_index],
               model.nodes[node_i].name.c_str());
    }

    free(gltf_material_indexing);
    free(gltf_mesh_indexing);
}
