#include <stdlib.h>
#include <string.h>

#include "bc7/bc7enc.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
#include "engine/platform/platform.h"
#include "gltf_baker.h"
#include "shared/scene_types.h"
#include "tools/core/mesh_utilities.h"

static uint64_t hash_data(const void* data, size_t length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t       hash  = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

static void parse_primitive_material(
    cgltf_primitive* prim,
    cgltf_data*      data,
    int32_t*         img_map,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    vec4_t*          out_base_color,
    cgltf_int*       out_uv_index,
    bool*            out_has_normal_map
) {
    *out_base_color     = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};
    *out_uv_index       = 0;
    *out_has_normal_map = false;

    if (!prim->material) {
        log_info("Primitive has NO Material attached.");
        return;
    }

    const char* mat_name = prim->material->name ? prim->material->name : "Unnamed";
    log_info("Primitive Material: %s", mat_name);

    if (prim->material->has_pbr_metallic_roughness) {
        cgltf_float* factor = prim->material->pbr_metallic_roughness.base_color_factor;
        *out_base_color     = (vec4_t){factor[0], factor[1], factor[2], factor[3]};

        cgltf_texture_view* bc_view = &prim->material->pbr_metallic_roughness.base_color_texture;
        if (bc_view->texture && bc_view->texture->image) {
            *out_uv_index                       = bc_view->texcoord;
            size_t img_idx                      = bc_view->texture->image - data->images;
            current_mesh->base_color_texture_id = img_map[img_idx];

            // Upgrade UNORM to SRGB for base color
            if (current_mesh->base_color_texture_id != -1) {
                pak_texture_format_t* fmt =
                    &out_scene->textures[current_mesh->base_color_texture_id].format;
                if (*fmt == PAK_TEX_FORMAT_PNG_UNORM)
                    *fmt = PAK_TEX_FORMAT_PNG_SRGB;
                else if (*fmt == PAK_TEX_FORMAT_RGBA8_UNORM)
                    *fmt = PAK_TEX_FORMAT_RGBA8_SRGB;
            }
            log_info(
                "  - Base Color Texture -> PAK ID %d (Upgraded to SRGB)",
                current_mesh->base_color_texture_id
            );
        }
    }

    cgltf_texture* nrm_tex = prim->material->normal_texture.texture;
    if (nrm_tex && nrm_tex->image) {
        size_t img_idx                  = nrm_tex->image - data->images;
        current_mesh->normal_texture_id = img_map[img_idx];
        *out_has_normal_map             = true;
        log_info(
            "  - Normal Map Texture -> PAK ID %d (Left as UNORM)", current_mesh->normal_texture_id
        );
    } else {
        log_warn("  ! WARNING: Material '%s' has NO Normal Map. Using flat fallback.", mat_name);
    }
}

static void parse_primitive_vertices(
    cgltf_primitive* prim,
    scene_desc_t*    out_scene,
    uint32_t         vertex_offset,
    mat4_t           global_transform,
    vec4_t           base_color,
    cgltf_int        uv_index
) {
    uint32_t vertex_count = prim->attributes[0].data->count;

    // PRE-CALCULATE MATRIX MATH ONCE PER MESH, NOT PER VERTEX
    float m00 = global_transform.data[0][0], m10 = global_transform.data[1][0],
          m20 = global_transform.data[2][0];
    float m01 = global_transform.data[0][1], m11 = global_transform.data[1][1],
          m21 = global_transform.data[2][1];
    float m02 = global_transform.data[0][2], m12 = global_transform.data[1][2],
          m22 = global_transform.data[2][2];

    // Cofactor matrix for correct normal transformation
    float c00 = m11 * m22 - m12 * m21, c01 = m02 * m21 - m01 * m22, c02 = m01 * m12 - m02 * m11;
    float c10 = m12 * m20 - m10 * m22, c11 = m00 * m22 - m02 * m20, c12 = m02 * m10 - m00 * m12;
    float c20 = m10 * m21 - m11 * m20, c21 = m01 * m20 - m00 * m21, c22 = m00 * m11 - m01 * m10;

    for (size_t v = 0; v < vertex_count; v++) {
        pak_vertex_t* out_vert = &out_scene->vertices[vertex_offset + v];

        vec3_t local_pos     = {0};
        vec3_t local_norm    = {0, 1, 0};
        vec4_t local_tangent = {1.0f, 0.0f, 0.0f, 0.0f};

        out_vert->uv    = (pak_uv_t){0.0f, 0.0f};
        out_vert->color = base_color;

        for (size_t a = 0; a < prim->attributes_count; a++) {
            cgltf_attribute* attr = &prim->attributes[a];
            if (attr->type == cgltf_attribute_type_position) {
                cgltf_accessor_read_float(attr->data, v, (float*)&local_pos, 3);
            } else if (attr->type == cgltf_attribute_type_normal) {
                cgltf_accessor_read_float(attr->data, v, (float*)&local_norm, 3);
            } else if (attr->type == cgltf_attribute_type_texcoord && attr->index == uv_index) {
                cgltf_accessor_read_float(attr->data, v, (float*)&out_vert->uv, 2);
            } else if (attr->type == cgltf_attribute_type_color && attr->index == 0) {
                cgltf_accessor_read_float(attr->data, v, (float*)&out_vert->color, 4);
            }
        }

        out_vert->pos.x = local_pos.x * m00 + local_pos.y * m10 + local_pos.z * m20 +
                          global_transform.data[3][0];
        out_vert->pos.y = local_pos.x * m01 + local_pos.y * m11 + local_pos.z * m21 +
                          global_transform.data[3][1];
        out_vert->pos.z = local_pos.x * m02 + local_pos.y * m12 + local_pos.z * m22 +
                          global_transform.data[3][2];

        float nx    = local_norm.x * c00 + local_norm.y * c01 + local_norm.z * c02;
        float ny    = local_norm.x * c10 + local_norm.y * c11 + local_norm.z * c12;
        float nz    = local_norm.x * c20 + local_norm.y * c21 + local_norm.z * c22;
        float n_len = sqrtf(nx * nx + ny * ny + nz * nz);

        if (n_len > 0.00001f) {
            nx /= n_len;
            ny /= n_len;
            nz /= n_len;
        }
        out_vert->normal = (vec3_t){nx, ny, nz};

        float tx    = local_tangent.x * m00 + local_tangent.y * m10 + local_tangent.z * m20;
        float ty    = local_tangent.x * m01 + local_tangent.y * m11 + local_tangent.z * m21;
        float tz    = local_tangent.x * m02 + local_tangent.y * m12 + local_tangent.z * m22;
        float t_len = sqrtf(tx * tx + ty * ty + tz * tz);

        if (t_len > 0.00001f) {
            tx /= t_len;
            ty /= t_len;
            tz /= t_len;
        }
        out_vert->tangent = (vec4_t){tx, ty, tz, local_tangent.w};
    }
}

static void parse_primitive_indices(
    cgltf_primitive* prim,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    uint32_t         prim_vertex_offset
) {
    if (!prim->indices)
        return;

    for (size_t i = 0; i < prim->indices->count; i++) {
        uint32_t raw_index                           = cgltf_accessor_read_index(prim->indices, i);
        out_scene->indices[out_scene->index_count++] = raw_index + (prim_vertex_offset -
                                                                    current_mesh->vertex_offset);
        current_mesh->index_count++;
    }
}

static void bake_gltf_mesh(
    cgltf_node*   node,
    mat4_t        global_transform,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    cgltf_data*   data,
    int32_t*      img_map
) {
    if (!node->mesh)
        return;

    for (size_t p = 0; p < node->mesh->primitives_count; p++) {
        cgltf_primitive* prim = &node->mesh->primitives[p];
        if (prim->attributes_count == 0)
            continue;

        if (out_scene->mesh_count >= PAK_MAX_MESHES ||
            out_scene->vertex_count + prim->attributes[0].data->count >= PAK_MAX_VERTICES) {
            log_error("Cooker out of memory! Too many meshes or vertices.");
            return;
        }

        pak_mesh_t* current_mesh                    = &out_scene->meshes[out_scene->mesh_count++];
        current_mesh->model_id                      = model_id;
        current_mesh->vertex_offset                 = out_scene->vertex_count;
        current_mesh->index_offset                  = out_scene->index_count;
        current_mesh->vertex_count                  = prim->attributes[0].data->count;
        current_mesh->index_count                   = 0;
        current_mesh->base_color_texture_id         = -1;
        current_mesh->normal_texture_id             = -1;
        current_mesh->metallic_roughness_texture_id = -1;

        uint32_t prim_vertex_offset = out_scene->vertex_count;
        out_scene->vertex_count += current_mesh->vertex_count;

        vec4_t    base_color;
        cgltf_int uv_index;
        bool      has_normal_map;
        parse_primitive_material(
            prim, data, img_map, out_scene, current_mesh, &base_color, &uv_index, &has_normal_map
        );
        parse_primitive_vertices(
            prim, out_scene, prim_vertex_offset, global_transform, base_color, uv_index
        );
        parse_primitive_indices(prim, out_scene, current_mesh, prim_vertex_offset);

        if (has_normal_map) {
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
                current_mesh->vertex_count,
                temp_indices,
                temp_index_count
            );

            if (temp_indices)
                free(temp_indices);
        }
    }
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
    bake_gltf_mesh(node, global_transform, out_scene, model_id, data, img_map);
    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(node->children[i], global_transform, out_scene, model_id, data, img_map);
    }
}

static inline void bc7enc_compress_block_params_init_linear_weights_gltf(
    bc7enc_compress_block_params* p
) {
    p->m_perceptual = BC7ENC_FALSE;
    p->m_weights[0] = 1;
    p->m_weights[1] = 1;
    p->m_weights[2] = 1;
    p->m_weights[3] = 1;
}

static inline void bc7enc_compress_block_params_init_perceptual_weights_gltf(
    bc7enc_compress_block_params* p
) {
    p->m_perceptual = BC7ENC_TRUE;
    p->m_weights[0] = 128;
    p->m_weights[1] = 64;
    p->m_weights[2] = 16;
    p->m_weights[3] = 32;
}

static inline void bc7enc_compress_block_params_init_gltf(bc7enc_compress_block_params* p) {
    p->m_max_partitions_mode                  = BC7ENC_MAX_PARTITIONS1;
    p->m_try_least_squares                    = BC7ENC_TRUE;
    p->m_mode_partition_estimation_filterbank = BC7ENC_TRUE;
    p->m_uber_level                           = 0;
    p->m_use_mode5_for_alpha                  = BC7ENC_TRUE;
    p->m_use_mode7_for_alpha                  = BC7ENC_TRUE;
    bc7enc_compress_block_params_init_perceptual_weights_gltf(p);
}

typedef struct {
    scene_desc_t* out_scene;
    cgltf_data*   data;
    int32_t*      img_map;
    uint32_t      total_images;
    bool          opt_fast_textures;

    platform_atomic_int_t next_job_idx;
} bake_context_t;

static void process_texture_job(bake_context_t* ctx, uint32_t gltf_idx, uint32_t tex_idx) {
    cgltf_image*  gltf_img  = &ctx->data->images[gltf_idx];
    scene_desc_t* out_scene = ctx->out_scene;

    void* src_data = (uint8_t*)gltf_img->buffer_view->buffer->data + gltf_img->buffer_view->offset;
    uint32_t compressed_size = gltf_img->buffer_view->size;

    if (!ctx->opt_fast_textures) {
        return; // Skip compression if fast textures are disabled
    }

    uint64_t data_hash = hash_data(src_data, compressed_size);
    char     cache_path[512];
    snprintf(
        cache_path, sizeof(cache_path), "./.cache/tex_%llx.bc7", (unsigned long long)data_hash
    );

    FILE* cache_file = fopen(cache_path, "rb");
    if (cache_file) {
        fseek(cache_file, 0, SEEK_END);
        long cached_size = ftell(cache_file);
        fseek(cache_file, 0, SEEK_SET);

        int w, h, channels;
        stbi_info_from_memory(src_data, compressed_size, &w, &h, &channels);

        out_scene->textures[tex_idx].byte_size   = cached_size;
        out_scene->textures[tex_idx].width       = w;
        out_scene->textures[tex_idx].height      = h;
        out_scene->textures[tex_idx].channels    = 4;
        out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_BC7_UNORM;
        out_scene->textures[tex_idx].byte_offset = 0;

        out_scene->raw_texture_bytes[tex_idx] = malloc(cached_size);
        fread(out_scene->raw_texture_bytes[tex_idx], 1, cached_size, cache_file);
        fclose(cache_file);

        log_info("  -> [CACHE HIT] Loaded BC7 instantly from %s", cache_path);
        return;
    }

    // Cache Miss: Compress
    int      w, h, channels;
    stbi_uc* raw_pixels = stbi_load_from_memory(src_data, compressed_size, &w, &h, &channels, 4);

    if (raw_pixels) {
        uint32_t blocks_x             = (w + 3) / 4;
        uint32_t blocks_y             = (h + 3) / 4;
        uint32_t compressed_byte_size = blocks_x * blocks_y * 16;

        out_scene->textures[tex_idx].byte_size   = compressed_byte_size;
        out_scene->textures[tex_idx].width       = w;
        out_scene->textures[tex_idx].height      = h;
        out_scene->textures[tex_idx].channels    = 4;
        out_scene->textures[tex_idx].format      = PAK_TEX_FORMAT_BC7_UNORM;
        out_scene->textures[tex_idx].byte_offset = 0;

        out_scene->raw_texture_bytes[tex_idx] = malloc(compressed_byte_size);
        uint8_t* bc7_dst                      = out_scene->raw_texture_bytes[tex_idx];

        bc7enc_compress_block_params pack_params;
        bc7enc_compress_block_params_init_gltf(&pack_params);
        bc7enc_compress_block_params_init_linear_weights_gltf(&pack_params);
        pack_params.m_max_partitions_mode = 0;
        pack_params.m_try_least_squares   = BC7ENC_FALSE;

        uint32_t dst_offset = 0;
        for (uint32_t by = 0; by < blocks_y; by++) {
            for (uint32_t bx = 0; bx < blocks_x; bx++) {
                uint32_t block_pixels[16] = {0};
                for (uint32_t py = 0; py < 4; py++) {
                    for (uint32_t px = 0; px < 4; px++) {
                        uint32_t global_x = bx * 4 + px;
                        uint32_t global_y = by * 4 + py;
                        global_x          = (global_x < (uint32_t)w) ? global_x : (uint32_t)w - 1;
                        global_y          = (global_y < (uint32_t)h) ? global_y : (uint32_t)h - 1;

                        uint32_t src_idx          = (global_y * w + global_x) * 4;
                        block_pixels[py * 4 + px] = ((uint32_t)raw_pixels[src_idx + 0] << 0) |
                                                    ((uint32_t)raw_pixels[src_idx + 1] << 8) |
                                                    ((uint32_t)raw_pixels[src_idx + 2] << 16) |
                                                    ((uint32_t)raw_pixels[src_idx + 3] << 24);
                    }
                }
                bc7enc_compress_block(&bc7_dst[dst_offset], block_pixels, &pack_params);
                dst_offset += 16;
            }
        }
        stbi_image_free(raw_pixels);

        FILE* write_cache = fopen(cache_path, "wb");
        if (write_cache) {
            fwrite(bc7_dst, 1, compressed_byte_size, write_cache);
            fclose(write_cache);
            log_info("  -> [CACHE MISS] Compressed & Saved to %s", cache_path);
        } else {
            log_warn("  -> Failed to write cache. Does .cache/ exist?");
        }
    }
}

static int texture_worker_thread(void* thread_data) {
    bake_context_t* ctx = (bake_context_t*)thread_data;

    while (true) {
        int i = platform_atomic_int_add(&ctx->next_job_idx, 1);

        if (i >= ctx->total_images) {
            break;
        }

        int32_t tex_idx = ctx->img_map[i];
        if (tex_idx != -1) {
            process_texture_job(ctx, i, (uint32_t)tex_idx);
        }
    }

    return 0;
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
    for (size_t i = 0; i < data->images_count; i++) {
        cgltf_image* gltf_img = &data->images[i];
        if (gltf_img->buffer_view != NULL && out_scene->texture_count < PAK_MAX_TEXTURES) {
            img_map[i] = out_scene->texture_count++;
            log_info("GLTF Image [%zu] -> Mapped to PAK Texture ID: %d", i, img_map[i]);
        } else {
            img_map[i] = -1;
        }
    }

    bake_context_t ctx = {
        .out_scene         = out_scene,
        .data              = data,
        .img_map           = img_map,
        .total_images      = data->images_count,
        .opt_fast_textures = opt_fast_textures
    };
    platform_atomic_int_set(&ctx.next_job_idx, 0);

    int               num_cores = platform_get_core_count();
    platform_thread_t threads[num_cores];

    for (int t = 0; t < num_cores; t++) {
        char thread_name[32];
        snprintf(thread_name, sizeof(thread_name), "CookerWorker_%d", t);
        threads[t] = platform_thread_create(texture_worker_thread, thread_name, &ctx);
    }

    for (int t = 0; t < num_cores; t++) {
        platform_thread_wait(threads[t]);
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
