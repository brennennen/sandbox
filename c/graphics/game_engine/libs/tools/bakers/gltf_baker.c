

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
#include "shared/scene_types.h"

#include "gltf_baker.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <string.h>

static void calculate_tangents(
    pak_vertex_t* vertices,
    uint32_t      vertex_count,
    uint32_t*     indices,
    uint32_t      index_count
) {
    for (uint32_t i = 0; i < vertex_count; i++) {
        vertices[i].tangent = (vec4_t){0.0f, 0.0f, 0.0f, 0.0f};
    }

    vec3_t*  bitangents     = calloc(vertex_count, sizeof(vec3_t));
    uint32_t triangle_count = index_count > 0 ? index_count / 3 : vertex_count / 3;

    for (uint32_t t = 0; t < triangle_count; t++) {
        uint32_t i0 = index_count > 0 ? indices[t * 3 + 0] : t * 3 + 0;
        uint32_t i1 = index_count > 0 ? indices[t * 3 + 1] : t * 3 + 1;
        uint32_t i2 = index_count > 0 ? indices[t * 3 + 2] : t * 3 + 2;

        pak_vertex_t* v0 = &vertices[i0];
        pak_vertex_t* v1 = &vertices[i1];
        pak_vertex_t* v2 = &vertices[i2];

        float e1x = v1->pos.data[0] - v0->pos.data[0];
        float e1y = v1->pos.data[1] - v0->pos.data[1];
        float e1z = v1->pos.data[2] - v0->pos.data[2];

        float e2x = v2->pos.data[0] - v0->pos.data[0];
        float e2y = v2->pos.data[1] - v0->pos.data[1];
        float e2z = v2->pos.data[2] - v0->pos.data[2];

        float duv1x = v1->uv.data[0] - v0->uv.data[0];
        float duv1y = v1->uv.data[1] - v0->uv.data[1];

        float duv2x = v2->uv.data[0] - v0->uv.data[0];
        float duv2y = v2->uv.data[1] - v0->uv.data[1];

        float f = 1.0f / (duv1x * duv2y - duv2x * duv1y);
        if (f > 999999.0f || f < -999999.0f)
            f = 1.0f;

        float tx = f * (duv2y * e1x - duv1y * e2x);
        float ty = f * (duv2y * e1y - duv1y * e2y);
        float tz = f * (duv2y * e1z - duv1y * e2z);

        float bx = f * (-duv2x * e1x + duv1x * e2x);
        float by = f * (-duv2x * e1y + duv1x * e2y);
        float bz = f * (-duv2x * e1z + duv1x * e2z);

        v0->tangent.data[0] += tx;
        v0->tangent.data[1] += ty;
        v0->tangent.data[2] += tz;
        v1->tangent.data[0] += tx;
        v1->tangent.data[1] += ty;
        v1->tangent.data[2] += tz;
        v2->tangent.data[0] += tx;
        v2->tangent.data[1] += ty;
        v2->tangent.data[2] += tz;

        bitangents[i0].data[0] += bx;
        bitangents[i0].data[1] += by;
        bitangents[i0].data[2] += bz;
        bitangents[i1].data[0] += bx;
        bitangents[i1].data[1] += by;
        bitangents[i1].data[2] += bz;
        bitangents[i2].data[0] += bx;
        bitangents[i2].data[1] += by;
        bitangents[i2].data[2] += bz;
    }

    for (uint32_t i = 0; i < vertex_count; i++) {
        vec3_t n = vertices[i].normal;
        vec4_t t = vertices[i].tangent;
        vec3_t b = bitangents[i];

        float dot_nt = n.data[0] * t.data[0] + n.data[1] * t.data[1] + n.data[2] * t.data[2];
        float ox     = t.data[0] - n.data[0] * dot_nt;
        float oy     = t.data[1] - n.data[1] * dot_nt;
        float oz     = t.data[2] - n.data[2] * dot_nt;

        float len = sqrtf(ox * ox + oy * oy + oz * oz);
        if (len > 0.0001f) {
            ox /= len;
            oy /= len;
            oz /= len;
        } else {
            ox = 1.0f;
            oy = 0.0f;
            oz = 0.0f;
        }

        float cx = n.data[1] * oz - n.data[2] * oy;
        float cy = n.data[2] * ox - n.data[0] * oz;
        float cz = n.data[0] * oy - n.data[1] * ox;

        float handedness = (cx * b.data[0] + cy * b.data[1] + cz * b.data[2]) < 0.0f ? -1.0f : 1.0f;

        vertices[i].tangent = (vec4_t){ox, oy, oz, handedness};
    }
    free(bitangents);
}

static void bake_gltf_node(
    cgltf_node*   node,
    mat4_t        parent_transform,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    cgltf_data*   data,
    int32_t*      img_map
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

            if (out_scene->mesh_count >= PAK_MAX_MESHES) {
                log_error("Cooker out of memory! Too many meshes.");
                return;
            }

            pak_mesh_t* current_mesh            = &out_scene->meshes[out_scene->mesh_count++];
            current_mesh->model_id              = model_id;
            current_mesh->vertex_offset         = out_scene->vertex_count;
            current_mesh->index_offset          = out_scene->index_count;
            current_mesh->vertex_count          = 0;
            current_mesh->index_count           = 0;
            current_mesh->base_color_texture_id = -1;
            current_mesh->normal_texture_id     = -1;
            current_mesh->metallic_roughness_texture_id = -1;

            uint32_t prim_vertex_offset = out_scene->vertex_count;
            uint32_t prim_vertex_count  = prim->attributes[0].data->count;

            if (out_scene->vertex_count + prim_vertex_count >= PAK_MAX_VERTICES) {
                log_error("Cooker out of memory! Too many vertices.");
                return;
            }

            vec4_t         base_color = {1.0f, 1.0f, 1.0f, 1.0f};
            cgltf_texture* tex_ptr    = NULL;
            cgltf_texture* nrm_tex    = NULL;
            cgltf_int      uv_index   = 0;

            if (prim->material) {
                const char* mat_name = prim->material->name ? prim->material->name : "Unnamed";
                log_info("Primitive Material: %s", mat_name);

                if (prim->material->has_pbr_metallic_roughness) {
                    cgltf_float* factor = prim->material->pbr_metallic_roughness.base_color_factor;
                    base_color          = (vec4_t){factor[0], factor[1], factor[2], factor[3]};

                    cgltf_texture_view* bc_view =
                        &prim->material->pbr_metallic_roughness.base_color_texture;
                    if (bc_view->texture && bc_view->texture->image) {
                        uv_index       = bc_view->texcoord;
                        size_t img_idx = bc_view->texture->image - data->images;
                        current_mesh->base_color_texture_id = img_map[img_idx];
                        if (current_mesh->base_color_texture_id != -1) {
                            pak_texture_format_t* fmt =
                                &out_scene->textures[current_mesh->base_color_texture_id].format;
                            if (*fmt == PAK_TEX_FORMAT_PNG_UNORM) {
                                *fmt = PAK_TEX_FORMAT_PNG_SRGB;
                            } else if (*fmt == PAK_TEX_FORMAT_RGBA8_UNORM) {
                                *fmt = PAK_TEX_FORMAT_RGBA8_SRGB;
                            }
                        }

                        log_info(
                            "  - Base Color Texture -> PAK ID %d (Upgraded to SRGB)",
                            current_mesh->base_color_texture_id
                        );
                    }
                }

                nrm_tex = prim->material->normal_texture.texture;
                if (nrm_tex && nrm_tex->image) {
                    size_t img_idx                  = nrm_tex->image - data->images;
                    current_mesh->normal_texture_id = img_map[img_idx];
                    log_info(
                        "  - Normal Map Texture -> PAK ID %d (Left as UNORM)",
                        current_mesh->normal_texture_id
                    );
                } else {
                    log_warn(
                        "  ! WARNING: Material '%s' has NO Normal Map. Using flat fallback.",
                        mat_name
                    );
                }
            } else {
                log_info("Primitive has NO Material attached.");
            }

            for (size_t v = 0; v < prim_vertex_count; v++) {
                pak_vertex_t* out_vert = &out_scene->vertices[out_scene->vertex_count++];

                vec3_t local_pos     = {0};
                vec3_t local_norm    = {0, 1, 0};
                vec4_t local_tangent = {1.0f, 0.0f, 0.0f, 0.0f};

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
                    // always recalculate tangents
                    // else if (attr->type == cgltf_attribute_type_tangent) {
                    //     cgltf_accessor_read_float(attr->data, v, (float*)&local_tangent, 4);
                    // }
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

                float m00 = global_transform.data[0][0];
                float m10 = global_transform.data[1][0];
                float m20 = global_transform.data[2][0];
                float m01 = global_transform.data[0][1];
                float m11 = global_transform.data[1][1];
                float m21 = global_transform.data[2][1];
                float m02 = global_transform.data[0][2];
                float m12 = global_transform.data[1][2];
                float m22 = global_transform.data[2][2];

                float it00 = m11 * m22 - m12 * m21;
                float it10 = m12 * m20 - m10 * m22;
                float it20 = m10 * m21 - m11 * m20;

                float it01 = m21 * m02 - m22 * m01;
                float it11 = m22 * m00 - m20 * m02;
                float it21 = m20 * m01 - m21 * m00;

                float it02 = m01 * m12 - m02 * m11;
                float it12 = m02 * m10 - m00 * m12;
                float it22 = m00 * m11 - m01 * m10;

                float c00 = m11 * m22 - m12 * m21;
                float c01 = m02 * m21 - m01 * m22;
                float c02 = m01 * m12 - m02 * m11;

                float c10 = m12 * m20 - m10 * m22;
                float c11 = m00 * m22 - m02 * m20;
                float c12 = m02 * m10 - m00 * m12;

                float c20 = m10 * m21 - m11 * m20;
                float c21 = m01 * m20 - m00 * m21;
                float c22 = m00 * m11 - m01 * m10;

                float nx = local_norm.x, ny = local_norm.y, nz = local_norm.z;
                float new_nx = nx * c00 + ny * c01 + nz * c02;
                float new_ny = nx * c10 + ny * c11 + nz * c12;
                float new_nz = nx * c20 + ny * c21 + nz * c22;
                float n_len  = sqrtf(new_nx * new_nx + new_ny * new_ny + new_nz * new_nz);
                if (n_len > 0.00001f) {
                    new_nx /= n_len;
                    new_ny /= n_len;
                    new_nz /= n_len;
                }
                out_vert->normal.x = new_nx;
                out_vert->normal.y = new_ny;
                out_vert->normal.z = new_nz;

                float tx = local_tangent.x, ty = local_tangent.y, tz = local_tangent.z;
                float new_tx = tx * m00 + ty * m10 + tz * m20;
                float new_ty = tx * m01 + ty * m11 + tz * m21;
                float new_tz = tx * m02 + ty * m12 + tz * m22;
                float t_len  = sqrtf(new_tx * new_tx + new_ty * new_ty + new_tz * new_tz);
                if (t_len > 0.00001f) {
                    new_tx /= t_len;
                    new_ty /= t_len;
                    new_tz /= t_len;
                }
                out_vert->tangent.x = new_tx;
                out_vert->tangent.y = new_ty;
                out_vert->tangent.z = new_tz;
                out_vert->tangent.w = local_tangent.w;
            }

            if (prim->indices) {
                if (out_scene->index_count + prim->indices->count >= PAK_MAX_INDICES)
                    return;
                for (size_t i = 0; i < prim->indices->count; i++) {
                    uint32_t raw_index = cgltf_accessor_read_index(prim->indices, i);
                    out_scene->indices[out_scene->index_count++] = raw_index +
                                                                   (prim_vertex_offset -
                                                                    current_mesh->vertex_offset);
                    current_mesh->index_count++;
                }
            }

            bool has_tangents = false;
            for (cgltf_size a = 0; a < prim->attributes_count; a++) {
                if (prim->attributes[a].type == cgltf_attribute_type_tangent) {
                    has_tangents = true;
                    break;
                }
            }

            if (nrm_tex != NULL) {
                uint32_t* temp_indices     = NULL;
                uint32_t  temp_index_count = 0;

                if (prim->indices) {
                    temp_index_count = prim->indices->count;
                    temp_indices     = malloc(temp_index_count * sizeof(uint32_t));
                    for (size_t i = 0; i < temp_index_count; i++) {
                        temp_indices[i] = cgltf_accessor_read_index(prim->indices, i);
                    }
                }

                calculate_tangents(
                    &out_scene->vertices[prim_vertex_offset],
                    prim_vertex_count,
                    temp_indices,
                    temp_index_count
                );

                if (temp_indices) {
                    free(temp_indices);
                }

                static bool has_logged = false;
                if (!has_logged && prim_vertex_count >= 3) {
                    log_info("TBN:");

                    for (int v = 0; v < 3; v++) {
                        pak_vertex_t* out_vert = &out_scene->vertices[prim_vertex_offset + v];

                        log_info("Vertex [%d]", v);
                        log_info(
                            "  Pos:     (%f, %f, %f)",
                            out_vert->pos.x,
                            out_vert->pos.y,
                            out_vert->pos.z
                        );
                        log_info("  UV:      (%f, %f)", out_vert->uv.u, out_vert->uv.v);
                        log_info(
                            "  Normal:  (%f, %f, %f)",
                            out_vert->normal.x,
                            out_vert->normal.y,
                            out_vert->normal.z
                        );
                        log_info(
                            "  Tangent: (%f, %f, %f, W: %f)",
                            out_vert->tangent.x,
                            out_vert->tangent.y,
                            out_vert->tangent.z,
                            out_vert->tangent.w
                        );

                        float nx = out_vert->normal.x, ny = out_vert->normal.y,
                              nz = out_vert->normal.z;
                        float tx = out_vert->tangent.x, ty = out_vert->tangent.y,
                              tz = out_vert->tangent.z;
                        float w  = out_vert->tangent.w;

                        float bx = (ny * tz - nz * ty) * w;
                        float by = (nz * tx - nx * tz) * w;
                        float bz = (nx * ty - ny * tx) * w;

                        log_info("  Derived B: (%f, %f, %f)", bx, by, bz);
                    }
                    log_info("========================================");
                    has_logged = true;
                }
            }

            current_mesh->vertex_count += prim_vertex_count;
        }
    }

    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(node->children[i], global_transform, out_scene, model_id, data, img_map);
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
    int32_t* img_map = malloc(data->images_count * sizeof(int32_t));
    for (size_t i = 0; i < data->images_count; i++)
        img_map[i] = -1;

    for (size_t i = 0; i < data->images_count; i++) {
        cgltf_image* gltf_img = &data->images[i];
        bool         has_bv   = gltf_img->buffer_view != NULL;

        log_info("GLTF Image [%zu]: has_buffer_view=%d", i, has_bv);
        if (has_bv) {
            if (out_scene->texture_count >= PAK_MAX_TEXTURES)
                break;

            uint32_t tex_idx = out_scene->texture_count++;
            img_map[i]       = tex_idx;
            log_info("  -> Successfully mapped to PAK Texture ID: %d", tex_idx);

            void* src_data = (uint8_t*)gltf_img->buffer_view->buffer->data +
                             gltf_img->buffer_view->offset;
            uint32_t compressed_size = gltf_img->buffer_view->size;

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
                    out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_RGBA8_UNORM;
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
                    out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_PNG_UNORM;
                    out_scene->textures[tex_idx].byte_offset = 0;

                    out_scene->raw_texture_bytes[tex_idx] = malloc(compressed_size);
                    memcpy(out_scene->raw_texture_bytes[tex_idx], src_data, compressed_size);
                }
            }
        }
    }

    cgltf_scene* gltf_scene = data->scene;
    if (!gltf_scene) {
        if (data->scenes_count > 0) {
            gltf_scene = &data->scenes[0];
        } else {
            log_error("CGLTF: No scenes found in %s", full_path);
            cgltf_free(data);
            free(img_map);
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
        bake_gltf_node(gltf_scene->nodes[i], root_transform, out_scene, model_id, data, img_map);
    }

    free(img_map);
    cgltf_free(data);
    return true;
}
