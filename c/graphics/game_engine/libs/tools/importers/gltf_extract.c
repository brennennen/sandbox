#include "gltf_extract.h"
#include "engine/core/logger.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sanitize_name(
    const char* in_name,
    char*       out_name,
    size_t      max_len,
    const char* fallback
) {
    if (!in_name || strlen(in_name) == 0) {
        strncpy(out_name, fallback, max_len);
        return;
    }
    strncpy(out_name, in_name, max_len);
    for (int i = 0; out_name[i]; i++) {
        if (out_name[i] == ' ' || out_name[i] == '/' || out_name[i] == '\\') {
            out_name[i] = '_';
        }
    }
}

static void extract_materials(cgltf_data* data, const char* output_dir) {
    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material* mat = &data->materials[i];

        char mat_name[128];
        char fallback[32];
        sprintf(fallback, "mat_%zu", i);
        sanitize_name(mat->name, mat_name, sizeof(mat_name), fallback);

        char filepath[256];
        sprintf(filepath, "%s/%s.mat", output_dir, mat_name);

        FILE* f = fopen(filepath, "w");
        if (!f)
            continue;

        fprintf(f, "Material {\n");

        if (mat->has_pbr_metallic_roughness) {
            cgltf_pbr_metallic_roughness* pbr = &mat->pbr_metallic_roughness;
            fprintf(
                f,
                "    base_color = [%f, %f, %f, %f];\n",
                pbr->base_color_factor[0],
                pbr->base_color_factor[1],
                pbr->base_color_factor[2],
                pbr->base_color_factor[3]
            );
            fprintf(f, "    roughness_factor = %f;\n", pbr->roughness_factor);
            fprintf(f, "    metallic_factor = %f;\n", pbr->metallic_factor);

            if (pbr->base_color_texture.texture) {
                fprintf(f, "    albedo_map = \"%s\";\n", "TODO_extract_image_path.png");
            }
        }

        fprintf(f, "}\n");
        fclose(f);
    }
}

static void extract_models(cgltf_data* data, const char* gltf_path, const char* output_dir) {
    for (size_t i = 0; i < data->meshes_count; i++) {
        cgltf_mesh* mesh = &data->meshes[i];

        char mesh_name[128];
        char fallback[32];
        sprintf(fallback, "mesh_%zu", i);
        sanitize_name(mesh->name, mesh_name, sizeof(mesh_name), fallback);

        char filepath[256];
        sprintf(filepath, "%s/%s.model", output_dir, mesh_name);

        FILE* f = fopen(filepath, "w");
        if (!f)
            continue;

        fprintf(f, "Model {\n");
        fprintf(f, "    source_file = \"%s\";\n", gltf_path);
        fprintf(f, "    mesh_name = \"%s\";\n", mesh->name);

        if (mesh->primitives_count > 0 && mesh->primitives[0].material) {
            char mat_name[128];
            sanitize_name(
                mesh->primitives[0].material->name, mat_name, sizeof(mat_name), "mat_fallback"
            );
            fprintf(f, "    materials = \"%s/%s.mat\";\n", output_dir, mat_name);
        }

        fprintf(f, "}\n");
        fclose(f);
    }
}

static void write_node_to_scene(FILE* f, cgltf_node* node, const char* output_dir) {
    if (node->mesh) {
        char mesh_name[128];
        sanitize_name(node->mesh->name, mesh_name, sizeof(mesh_name), "mesh_fallback");

        fprintf(f, "    Entity {\n");
        fprintf(f, "        name = \"%s\";\n", node->name ? node->name : mesh_name);
        fprintf(f, "        template_path = \"%s/%s.model\";\n", output_dir, mesh_name);

        if (node->has_translation) {
            fprintf(
                f,
                "        position = [%f, %f, %f];\n",
                node->translation[0],
                node->translation[1],
                node->translation[2]
            );
        }
        if (node->has_scale) {
            fprintf(
                f, "        scale = [%f, %f, %f];\n", node->scale[0], node->scale[1], node->scale[2]
            );
        }

        if (node->has_rotation) {
            fprintf(
                f,
                "        rotation = [%f, %f, %f, %f];\n",
                node->rotation[0],
                node->rotation[1],
                node->rotation[2],
                node->rotation[3]
            );
        }

        fprintf(f, "    }\n");
    }

    for (size_t i = 0; i < node->children_count; i++) {
        write_node_to_scene(f, node->children[i], output_dir);
    }
}

bool extract_gltf_monolithic(const char* gltf_path, const char* output_dir) {
    cgltf_options options = {0};
    cgltf_data*   data    = NULL;
    cgltf_result  result  = cgltf_parse_file(&options, gltf_path, &data);

    if (result != cgltf_result_success) {
        log_error("Failed to parse glTF: %s", gltf_path);
        return false;
    }

    log_info("Extracting %s to %s...", gltf_path, output_dir);

    extract_materials(data, output_dir);
    extract_models(data, gltf_path, output_dir);

    char scene_path[256];
    sprintf(scene_path, "%s/scene.scene", output_dir);

    FILE* f = fopen(scene_path, "w");
    if (f) {
        fprintf(f, "Scene {\n");
        for (size_t i = 0; i < data->scenes[0].nodes_count; i++) {
            write_node_to_scene(f, data->scenes[0].nodes[i], output_dir);
        }
        fprintf(f, "}\n");
        fclose(f);
    }

    cgltf_free(data);
    return true;
}
