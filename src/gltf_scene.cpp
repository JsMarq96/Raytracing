#include "gltf_scene.h"
#include "gl3w.h"
#include "glcorearb.h"
#include "material.h"
#include <cstdint>

#include "imgui/imgui.h"

void sScene::init() {
    memset(enabled, false, sizeof(sScene::enabled));
    memset(submesh_child, 0, sizeof(sScene::submesh_child));
    memset(is_submesh_full, false, sizeof(sScene::is_submesh_full));
    memset(is_material_full, false, sizeof(sScene::is_material_full));

    material_name_index_storage.init();
    node_name_index_storage.init();
};

void sScene::render(const sCamera &camera,
                    const sMat44 &view_proj,
                    const sVector3 &light_position) const {
    int render_mode = 0;

    ImGui::SliderInt("Render output mode", &render_mode, 0, 4);

    for(uint16_t node_i = 0; node_i < MAX_NODE_COUNT; node_i++) {
        if (!enabled[node_i]) {
            continue;
        }

        for(uint16_t submesh_index = mesh_of_object[node_i]; submesh_index < MAX_SUBMESH_COUNT ;) {
            const sSubMeshRenderData *render_data = &submeshes_render[submesh_index];
            // Bind VAO
            //std::cout << "> " <<  render_data->VAO << std::endl;
            glBindVertexArray(render_data->VAO);

            // Bind material
            const sMaterial *curr_mat = &materials[submesh_material[submesh_index]];

            curr_mat->enable();

            // TODO: uniforms
            curr_mat->shader.set_uniform_matrix4("u_model_mat", models[node_i]);
            curr_mat->shader.set_uniform_matrix4("u_viewproj_mat", view_proj);
            curr_mat->shader.set_uniform_vector("u_camera_position", camera.position);
            curr_mat->shader.set_uniform_vector("u_light_position", light_position);
            curr_mat->shader.set_uniform("u_output_mode", 0.0f + render_mode);

            glDrawElements(render_data->render_mode,
                            render_data->indices_size,
                            render_data->indices_type,
                            0);

            curr_mat->disable();

            // Render all the child/ associated submeshes with the current mesh
            if (submesh_child[submesh_index].has_child) {
                submesh_index = submesh_child[submesh_index].child_index;
            } else {
                break;
            }
        }
    }
}
