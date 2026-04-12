

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
#include "tools/core/scene_types.h"

#include "gltf_baker.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <string.h>

static void bake_gltf_node(
    cgltf_node*   node,
    mat4_t        parent_transform,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    cgltf_data*   data,
    int32_t*      tex_map
) {
    mat4_t local_transform = mat4_identity();
    if (node->has_matrix || node->has_translation || node->has_rotation || node->has_scale) {
        cgltf_float matrix[16];
        cgltf_node_transform_local(node, matrix);
        memcpy(&local_transform, matrix, sizeof(mat4_t));
    }

    mat4_t global_transform = mat4_mul(local_transform, parent_transform);

    if (node->mesh) {
        for (size_t p = 0; p < node->mesh->primitives_count; p++) {
            cgltf_primitive* prim = &node->mesh->primitives[p];

            if (prim->attributes_count == 0)
                continue;

            if (out_scene->mesh_count >= MAX_MESHES) {
                log_error("Cooker out of memory! Too many meshes.");
                return;
            }

            pak_mesh_t* current_mesh    = &out_scene->meshes[out_scene->mesh_count++];
            current_mesh->model_id      = model_id;
            current_mesh->vertex_offset = out_scene->vertex_count;
            current_mesh->index_offset  = out_scene->index_count;
            current_mesh->vertex_count  = 0;
            current_mesh->index_count   = 0;
            current_mesh->texture_id    = -1;

            uint32_t prim_vertex_offset = out_scene->vertex_count;
            uint32_t prim_vertex_count  = prim->attributes[0].data->count;

            if (out_scene->vertex_count + prim_vertex_count >= MAX_VERTICES) {
                log_error("Cooker out of memory! Too many vertices.");
                return;
            }

            vec4_t         base_color = {1.0f, 1.0f, 1.0f, 1.0f};
            cgltf_texture* tex_ptr    = NULL;
            cgltf_int      uv_index   = 0;

            if (prim->material) {
                const char* mat_name = prim->material->name ? prim->material->name : "Unnamed";
                log_info("Primitive Material: %s", mat_name);

                if (prim->material->has_pbr_metallic_roughness) {
                    cgltf_float* factor = prim->material->pbr_metallic_roughness.base_color_factor;
                    base_color          = (vec4_t){factor[0], factor[1], factor[2], factor[3]};
                    tex_ptr  = prim->material->pbr_metallic_roughness.base_color_texture.texture;
                    uv_index = prim->material->pbr_metallic_roughness.base_color_texture.texcoord;
                    log_info(
                        "  - Type: Metallic/Roughness | Color: [%.2f, %.2f, %.2f, %.2f]",
                        factor[0],
                        factor[1],
                        factor[2],
                        factor[3]
                    );
                } else if (prim->material->has_pbr_specular_glossiness) {
                    cgltf_float* factor = prim->material->pbr_specular_glossiness.diffuse_factor;
                    base_color          = (vec4_t){factor[0], factor[1], factor[2], factor[3]};
                    tex_ptr  = prim->material->pbr_specular_glossiness.diffuse_texture.texture;
                    uv_index = prim->material->pbr_specular_glossiness.diffuse_texture.texcoord;
                    log_info("  - Type: Specular/Glossiness");
                }

                if (tex_ptr) {
                    size_t local_idx         = tex_ptr - data->textures;
                    current_mesh->texture_id = tex_map[local_idx];
                    log_info(
                        "  - Found Texture: GLTF Index %zu -> PAK ID %d (UV Channel %d)",
                        local_idx,
                        current_mesh->texture_id,
                        uv_index
                    );
                } else {
                    log_info("  - NO Texture Image (Relies purely on Base Color)");
                }
            } else {
                log_info("Primitive has NO Material attached.");
            }

            for (size_t v = 0; v < prim_vertex_count; v++) {
                vertex_t* out_vert = &out_scene->vertices[out_scene->vertex_count++];

                vec3_t local_pos  = {0};
                vec3_t local_norm = {0, 1, 0};

                out_vert->uv.u  = 0.0f;
                out_vert->uv.v  = 0.0f;
                out_vert->color = base_color;

                for (size_t a = 0; a < prim->attributes_count; a++) {
                    cgltf_attribute* attr = &prim->attributes[a];
                    if (attr->type == cgltf_attribute_type_position) {
                        cgltf_accessor_read_float(attr->data, v, (float*)&local_pos, 3);
                    } else if (attr->type == cgltf_attribute_type_normal) {
                        cgltf_accessor_read_float(attr->data, v, (float*)&local_norm, 3);
                    } else if (attr->type == cgltf_attribute_type_texcoord &&
                               attr->index == uv_index) {
                        cgltf_accessor_read_float(attr->data, v, (float*)&out_vert->uv, 2);
                    } else if (attr->type == cgltf_attribute_type_color && attr->index == 0) {
                        cgltf_accessor_read_float(attr->data, v, (float*)&out_vert->color, 4);
                    }
                }

                float px = local_pos.x, py = local_pos.y, pz = local_pos.z;
                out_vert->pos.x = px * global_transform.data[0][0] +
                                  py * global_transform.data[1][0] +
                                  pz * global_transform.data[2][0] + global_transform.data[3][0];
                out_vert->pos.y = px * global_transform.data[0][1] +
                                  py * global_transform.data[1][1] +
                                  pz * global_transform.data[2][1] + global_transform.data[3][1];
                out_vert->pos.z = px * global_transform.data[0][2] +
                                  py * global_transform.data[1][2] +
                                  pz * global_transform.data[2][2] + global_transform.data[3][2];

                float nx = local_norm.x, ny = local_norm.y, nz = local_norm.z;
                out_vert->normal.x = nx * global_transform.data[0][0] +
                                     ny * global_transform.data[1][0] +
                                     nz * global_transform.data[2][0];
                out_vert->normal.y = nx * global_transform.data[0][1] +
                                     ny * global_transform.data[1][1] +
                                     nz * global_transform.data[2][1];
                out_vert->normal.z = nx * global_transform.data[0][2] +
                                     ny * global_transform.data[1][2] +
                                     nz * global_transform.data[2][2];
            }

            if (prim->indices) {
                if (out_scene->index_count + prim->indices->count >= MAX_INDICES)
                    return;
                for (size_t i = 0; i < prim->indices->count; i++) {
                    uint32_t raw_index = cgltf_accessor_read_index(prim->indices, i);
                    out_scene->indices[out_scene->index_count++] = raw_index +
                                                                   (prim_vertex_offset -
                                                                    current_mesh->vertex_offset);
                    current_mesh->index_count++;
                }
            }
            current_mesh->vertex_count += prim_vertex_count;
        }
    }

    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(node->children[i], global_transform, out_scene, model_id, data, tex_map);
    }
}

bool bake_model(
    const char*   full_path,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    bool          opt_fast_textures,
    bool          opt_z_up
) {
    cgltf_options options = {0};
    cgltf_data*   data    = NULL;

    if (cgltf_parse_file(&options, full_path, &data) != cgltf_result_success) {
        log_error("CGLTF Failed to parse model: %s", full_path);
        return false;
    }

    cgltf_load_buffers(&options, data, full_path);

    // Build the texture map
    int32_t* tex_map = malloc(data->textures_count * sizeof(int32_t));
    for (size_t i = 0; i < data->textures_count; i++)
        tex_map[i] = -1;

    // Process textures
    for (size_t t = 0; t < data->textures_count; t++) {
        cgltf_texture* gltf_tex = &data->textures[t];
        bool           has_img  = gltf_tex->image != NULL;
        bool           has_bv   = has_img && (gltf_tex->image->buffer_view != NULL);
        log_info("GLTF Texture [%zu]: has_image=%d, has_buffer_view=%d", t, has_img, has_bv);
        if (gltf_tex->image && gltf_tex->image->buffer_view) {
            if (out_scene->texture_count >= MAX_TEXTURES)
                break;

            uint32_t tex_idx = out_scene->texture_count++;
            tex_map[t]       = tex_idx;
            log_info("  -> Successfully mapped to PAK Texture ID: %d", tex_idx);

            void* src_data = (uint8_t*)gltf_tex->image->buffer_view->buffer->data +
                             gltf_tex->image->buffer_view->offset;
            uint32_t compressed_size = gltf_tex->image->buffer_view->size;

            if (opt_fast_textures) {
                int      w, h, channels;
                stbi_uc* raw_pixels = stbi_load_from_memory(
                    src_data, compressed_size, &w, &h, &channels, 4
                );

                if (raw_pixels) {
                    out_scene->textures[tex_idx].byte_size   = w * h * 4;
                    out_scene->textures[tex_idx].width       = w;
                    out_scene->textures[tex_idx].height      = h;
                    out_scene->textures[tex_idx].channels    = 4;
                    out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_RGBA8;
                    out_scene->textures[tex_idx].byte_offset = 0;
                    out_scene->raw_texture_bytes[tex_idx]    = raw_pixels;
                }
            } else {
                int w, h, channels;
                if (stbi_info_from_memory(src_data, compressed_size, &w, &h, &channels)) {
                    out_scene->textures[tex_idx].byte_size   = compressed_size;
                    out_scene->textures[tex_idx].width       = w;
                    out_scene->textures[tex_idx].height      = h;
                    out_scene->textures[tex_idx].channels    = 4;
                    out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_PNG;
                    out_scene->textures[tex_idx].byte_offset = 0;

                    out_scene->raw_texture_bytes[tex_idx] = malloc(compressed_size);
                    memcpy(out_scene->raw_texture_bytes[tex_idx], src_data, compressed_size);
                }
            }
        }
    }

    // Process scene hierarchy
    cgltf_scene* gltf_scene = data->scene;
    if (!gltf_scene) {
        if (data->scenes_count > 0) {
            gltf_scene = &data->scenes[0];
        } else {
            log_error("CGLTF: No scenes found in %s", full_path);
            cgltf_free(data);
            free(tex_map);
            return false;
        }
    }

    mat4_t root_transform = mat4_identity();
    if (opt_z_up) {
        root_transform.data[1][1] = 0.0f;
        root_transform.data[1][2] = 1.0f;
        root_transform.data[2][1] = -1.0f;
        root_transform.data[2][2] = 0.0f;
    }

    for (size_t i = 0; i < gltf_scene->nodes_count; i++) {
        bake_gltf_node(gltf_scene->nodes[i], root_transform, out_scene, model_id, data, tex_map);
    }

    free(tex_map);
    cgltf_free(data);
    return true;
}
