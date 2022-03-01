#include "../gltf_parser.h"
#include "glcorearb.h"
#include <cstdint>

void _upload_texture(const eTextureType text_type,
                     const tinygltf::Image *tiny_img,
                           sMaterial *material) {
        GLenum img_format = 0;
        switch (tiny_img->component) {
            case 1:
                img_format = GL_RED;
                break;
            case 2:
                img_format = GL_RG;
                break;
            case 3:
                img_format = GL_RGB;
                break;
            case 4:
                img_format = GL_RGBA;
                break;
            default:
                assert("Unsorported texture format AKA more than 4 dims (RGBA)");
        }

        GLenum data_size = 0;
        switch(tiny_img->bits) {
            case 8:
                data_size = GL_UNSIGNED_BYTE;
                break;
            case 16:
                data_size = GL_UNSIGNED_SHORT;
                break;
            default:
                assert("Unsupported texture datatype size (not an Unsigned byte and short)");
        }

        material->add_raw_texture((const char*) &tiny_img->image.at(0),
                                    tiny_img->width,
                                    tiny_img->height,
                                    img_format,
                                    data_size,
                                    text_type);
}
#include <bitset>
uint32_t* Parser::_load_gltf_materials(sScene *scene,
                                       const tinygltf::Model &model) {
    uint32_t *material_gltf_scene_indexing = (uint32_t*) malloc(sizeof(uint32_t) * model.materials.size());
    for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
        uint16_t material_index = 0;
        for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
            if (!scene->is_material_full[material_index]) {
                break;
            }
        }

        material_gltf_scene_indexing[material_i] = material_index;

        scene->is_material_full[material_index] = true;

        const tinygltf::Material *tiny_material = &model.materials[material_i];
        sMaterial *material = &scene->materials[material_index];

        // Store Material name
        scene->material_name_index_storage.add(tiny_material->name.c_str(),
                                               tiny_material->name.size() + 1,
                                               material_index);

        // TODO: AGUEIFNEI auto... sorry
        auto end_values = tiny_material->values.end();
        auto end_additional_values = tiny_material->additionalValues.end();

        auto it = tiny_material->values.find("baseColorTexture");
        if (it != end_values) {
            const tinygltf::Image *tiny_img = &model.images[model.textures[it->second.TextureIndex()].source];

            _upload_texture(COLOR_MAP,
                            tiny_img,
                            material);
        }

        it = tiny_material->additionalValues.find("normalTexture");
        if (it != end_additional_values) {
            const tinygltf::Image *tiny_img = &model.images[model.textures[it->second.TextureIndex()].source];

            _upload_texture(NORMAL_MAP,
                            tiny_img,
                            material);
        }
        it = tiny_material->values.find("metallicRoughnessTexture");
        if (it != end_values) {
            const tinygltf::Image *tiny_img = &model.images[model.textures[it->second.TextureIndex()].source];

            _upload_texture(METALLIC_ROUGHNESS_MAP,
                            tiny_img,
                            material);
        }

        uint8_t enabled_textures = material->get_used_textures();
        switch (enabled_textures) {
            case 0b1011:
                material->add_shader("resources/shaders/pbr.vs",
                                     "resources/shaders/pbr.fs");
                break;
            default:
                material->add_shader("resources/shaders/plain.vs",
                                     "resources/shaders/plain.fs");
        }
    }

    return material_gltf_scene_indexing;
}
