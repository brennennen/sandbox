#include <stdlib.h>
#include <string.h>

#include "bc7/bc7enc.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/platform/platform.h"
#include "gltf_baker.h"
#include "libs/core/resources/image.h"
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

typedef struct {
    uint8_t* pixels;
    int      width, height, channels;
} raw_image_t;

raw_image_t pack_orm_texture(raw_image_t* ao_map, raw_image_t* mr_map) {
    // Determine the final maximum resolution
    int width  = 1;
    int height = 1;
    if (mr_map && mr_map->width > width)
        width = mr_map->width;
    if (ao_map && ao_map->width > width)
        width = ao_map->width;
    if (mr_map && mr_map->height > height)
        height = mr_map->height;
    if (ao_map && ao_map->height > height)
        height = ao_map->height;

    raw_image_t packed;
    packed.width    = width;
    packed.height   = height;
    packed.channels = 4;

    // Allocate directly on the OS heap!
    packed.pixels = malloc(width * height * 4);

    if (packed.pixels == NULL) {
        log_error(
            "CRITICAL: System Out of Memory! Failed to allocate %d bytes for ORM map.",
            width * height * 4
        );
        return packed; // Return the struct with NULL pixels to avoid crashing
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (float)x / (float)width;
            float v = (float)y / (float)height;

            // OCCLUSION (Target: RED)
            uint8_t ao = 255; // Default to 1.0 (No shadow)
            if (ao_map && ao_map->pixels) {
                int src_x   = (int)(u * ao_map->width);
                int src_y   = (int)(v * ao_map->height);
                int src_idx = (src_y * ao_map->width + src_x) * ao_map->channels;
                ao          = ao_map->pixels[src_idx + 0]; // glTF AO is in Red channel
            }

            // ROUGHNESS & METALLIC (Target: GREEN & BLUE)
            uint8_t r = 255; // Default to 1.0 (Fully rough)
            uint8_t m = 0;   // Default to 0.0 (Non-metal)
            if (mr_map && mr_map->pixels) {
                int src_x   = (int)(u * mr_map->width);
                int src_y   = (int)(v * mr_map->height);
                int src_idx = (src_y * mr_map->width + src_x) * mr_map->channels;

                // glTF Roughness is Green, Metallic is Blue
                // Ensure the map actually has 3 or 4 channels before reading Green/Blue
                if (mr_map->channels >= 3) {
                    r = mr_map->pixels[src_idx + 1];
                    m = mr_map->pixels[src_idx + 2];
                }
            }
            // WRITE OUT
            int out_idx                = (y * width + x) * 4;
            packed.pixels[out_idx + 0] = ao;
            packed.pixels[out_idx + 1] = r;
            packed.pixels[out_idx + 2] = m;
            packed.pixels[out_idx + 3] = 255;
        }
    }

    return packed;
}

static void parse_primitive_material(
    arena_t*         scratch_arena,
    cgltf_primitive* primitive,
    cgltf_data*      data,
    int32_t*         img_map,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    vec4_t*          out_base_color,
    cgltf_int*       out_uv_index,
    bool*            out_has_normal_map,
    bool*            out_is_alpha_masked
) {
    *out_base_color      = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};
    *out_uv_index        = 0;
    *out_has_normal_map  = false;
    *out_is_alpha_masked = true;

    if (!primitive->material) {
        log_info("Primitive has NO Material attached.");
        return;
    }

    const char* alpha_mode_str = "UNKNOWN";
    switch (primitive->material->alpha_mode) {
    case cgltf_alpha_mode_opaque:
        alpha_mode_str = "OPAQUE";
        break;
    case cgltf_alpha_mode_mask:
        alpha_mode_str = "MASK";
        break;
    case cgltf_alpha_mode_blend:
        alpha_mode_str = "BLEND";
        break;
    case cgltf_alpha_mode_max_enum:
    default:
        break;
    }

    const char* mat_name = primitive->material->name ? primitive->material->name : "Unnamed";
    log_info("Primitive Material: '%s' | Alpha Mode: %s", mat_name, alpha_mode_str);

    if (primitive->material->alpha_mode == cgltf_alpha_mode_mask ||
        primitive->material->alpha_mode == cgltf_alpha_mode_blend) {
        *out_is_alpha_masked = true;
    }

    if (primitive->material->has_pbr_metallic_roughness) {
        cgltf_pbr_metallic_roughness* cgltf_pbr = &primitive->material->pbr_metallic_roughness;
        cgltf_float* factor = primitive->material->pbr_metallic_roughness.base_color_factor;
        *out_base_color     = (vec4_t){factor[0], factor[1], factor[2], factor[3]};
        current_mesh->metallic_factor  = cgltf_pbr->metallic_factor;
        current_mesh->roughness_factor = cgltf_pbr->roughness_factor;

        log_info(
            "  - PBR Factors -> Metallic: %.2f | Roughness: %.2f",
            current_mesh->metallic_factor,
            current_mesh->roughness_factor
        );

        cgltf_texture_view* base_color_view =
            &primitive->material->pbr_metallic_roughness.base_color_texture;

        if (base_color_view->texture && base_color_view->texture->image) {
            *out_uv_index                       = base_color_view->texcoord;
            size_t img_idx                      = base_color_view->texture->image - data->images;
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

        cgltf_texture_view* mr_view =
            &primitive->material->pbr_metallic_roughness.metallic_roughness_texture;
        cgltf_texture_view* ao_view = &primitive->material->occlusion_texture;

        int32_t mr_id = (mr_view->texture && mr_view->texture->image)
                            ? img_map[mr_view->texture->image - data->images]
                            : -1;

        int32_t ao_id = (ao_view->texture && ao_view->texture->image)
                            ? img_map[ao_view->texture->image - data->images]
                            : -1;

        // Evaluate and Pack
        if (mr_id == -1 && ao_id == -1) {
            current_mesh->ao_roughness_metallic_texture_id = -1;
            log_warn("  ! WARNING: Material '%s' has NO MR or AO maps.", mat_name);

        } else if (mr_id == ao_id && mr_id != -1) {
            // The artist already combined them into a single file!
            current_mesh->ao_roughness_metallic_texture_id = mr_id;
            log_info("  - ORM Map -> PAK ID %d (Already Packed)", mr_id);

        } else {
            log_info("  - Separated AO/MR maps detected. Synthesizing packed ORM...");

            // Extract raw data from your staging scene
            raw_image_t ao_raw = {0};
            if (ao_id != -1) {
                ao_raw.width    = out_scene->textures[ao_id].width;
                ao_raw.height   = out_scene->textures[ao_id].height;
                ao_raw.channels = out_scene->textures[ao_id].channels;
                // ao_raw.pixels   = out_scene->textures[ao_id].pixel_data;
                ao_raw.pixels = out_scene->raw_texture_bytes[ao_id];
            }

            raw_image_t mr_raw = {0};
            if (mr_id != -1) {
                mr_raw.width    = out_scene->textures[mr_id].width;
                mr_raw.height   = out_scene->textures[mr_id].height;
                mr_raw.channels = out_scene->textures[mr_id].channels;
                // mr_raw.pixels   = out_scene->textures[mr_id].pixel_data;
                mr_raw.pixels = out_scene->raw_texture_bytes[mr_id];
            }

            log_info("  - Debug ORM: mr_id=%d, ao_id=%d", mr_id, ao_id);

            if (ao_id != -1 && out_scene->raw_texture_bytes[ao_id] == NULL) {
                log_error(
                    "CRITICAL: AO Texture ID %d has a NULL pointer! Did the cache skip CPU "
                    "loading?",
                    ao_id
                );
                return; // Prevent the crash
            }
            if (mr_id != -1 && out_scene->raw_texture_bytes[mr_id] == NULL) {
                log_error(
                    "CRITICAL: MR Texture ID %d has a NULL pointer! Did the cache skip CPU "
                    "loading?",
                    mr_id
                );
                return; // Prevent the crash
            }

            // raw_image_t packed = pack_orm_texture(
            //     scratch_arena, (ao_id != -1) ? &ao_raw : NULL, (mr_id != -1) ? &mr_raw : NULL
            // );
            raw_image_t packed = pack_orm_texture(
                (ao_id != -1) ? &ao_raw : NULL, (mr_id != -1) ? &mr_raw : NULL
            );

            // Append the new synthesized texture to the scene's texture array
            uint32_t new_tex_idx = out_scene->texture_count++;

            out_scene->textures[new_tex_idx].width    = packed.width;
            out_scene->textures[new_tex_idx].height   = packed.height;
            out_scene->textures[new_tex_idx].channels = packed.channels;
            // out_scene->textures[new_tex_idx].pixel_data = packed.pixels;
            out_scene->textures[new_tex_idx].byte_size = packed.width * packed.height *
                                                         packed.channels;
            out_scene->textures[new_tex_idx].mip_levels = 1;
            out_scene->textures[new_tex_idx].format     = PAK_TEX_FORMAT_RGBA8_UNORM;

            out_scene->raw_texture_bytes[new_tex_idx] = packed.pixels;

            // Bind the newly created texture to the mesh
            current_mesh->ao_roughness_metallic_texture_id = new_tex_idx;
            log_info("  - Synthesized ORM Map -> PAK ID %d", new_tex_idx);
        }

        // cgltf_texture_view* ao_metallic_roughness_view =
        //     &primitive->material->pbr_metallic_roughness.metallic_roughness_texture;
        // if (ao_metallic_roughness_view->texture && ao_metallic_roughness_view->texture->image) {
        //     size_t img_idx = ao_metallic_roughness_view->texture->image - data->images;
        //     current_mesh->ao_roughness_metallic_texture_id = img_map[img_idx];

        //     log_info(
        //         "  - MR Map Texture -> PAK ID %d (Left as UNORM)",
        //         current_mesh->ao_roughness_metallic_texture_id
        //     );
        // } else {
        //     log_warn("  ! WARNING: Material '%s' has NO Metallic/Roughness Map.", mat_name);
        // }
    } else {
        // Fallback for meshes that completely lack a PBR material definition
        current_mesh->metallic_factor  = 0.0f; // Default to non-metal
        current_mesh->roughness_factor = 1.0f; // Default to fully rough (matte)
        log_warn("  ! WARNING: Material '%s' is not PBR. Using fallback factors.", mat_name);
    }

    cgltf_texture* nrm_tex = primitive->material->normal_texture.texture;
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
    cgltf_primitive* primitive,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    uint32_t         vertex_offset,
    mat4_t           global_transform,
    vec4_t           base_color,
    cgltf_int        uv_index
) {
    uint32_t vertex_count = primitive->attributes[0].data->count;
    mat3_t   cofactor     = mat4_get_normal_matrix(global_transform);
    vec3_t   min_bounds   = {FLT_MAX, FLT_MAX, FLT_MAX};
    vec3_t   max_bounds   = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (size_t v = 0; v < vertex_count; v++) {
        pak_vertex_t* out_vert = &out_scene->vertices[vertex_offset + v];

        vec3_t local_pos     = {0};
        vec3_t local_norm    = {0, 1, 0};
        vec4_t local_tangent = {1.0f, 0.0f, 0.0f, 0.0f};

        out_vert->uv    = (pak_uv_t){0.0f, 0.0f};
        out_vert->color = base_color;

        for (size_t a = 0; a < primitive->attributes_count; a++) {
            cgltf_attribute* attr = &primitive->attributes[a];
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

        out_vert->pos     = mat4_transform_point(global_transform, local_pos);
        out_vert->normal  = mat3_transform_normal(cofactor, local_norm);
        out_vert->tangent = mat4_transform_tangent(global_transform, local_tangent);

        if (out_vert->pos.x < min_bounds.x)
            min_bounds.x = out_vert->pos.x;
        if (out_vert->pos.y < min_bounds.y)
            min_bounds.y = out_vert->pos.y;
        if (out_vert->pos.z < min_bounds.z)
            min_bounds.z = out_vert->pos.z;

        if (out_vert->pos.x > max_bounds.x)
            max_bounds.x = out_vert->pos.x;
        if (out_vert->pos.y > max_bounds.y)
            max_bounds.y = out_vert->pos.y;
        if (out_vert->pos.z > max_bounds.z)
            max_bounds.z = out_vert->pos.z;
    }

    current_mesh->bounding_center = (vec3_t){(min_bounds.x + max_bounds.x) * 0.5f,
                                             (min_bounds.y + max_bounds.y) * 0.5f,
                                             (min_bounds.z + max_bounds.z) * 0.5f};

    vec3_t diff = {
        max_bounds.x - current_mesh->bounding_center.x,
        max_bounds.y - current_mesh->bounding_center.y,
        max_bounds.z - current_mesh->bounding_center.z
    };
    current_mesh->bounding_radius = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

static void parse_primitive_indices(
    cgltf_primitive* primitive,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    uint32_t         prim_vertex_offset
) {
    if (!primitive->indices)
        return;

    for (size_t i = 0; i < primitive->indices->count; i++) {
        uint32_t raw_index = cgltf_accessor_read_index(primitive->indices, i);
        out_scene->indices[out_scene->index_count++] = raw_index + (prim_vertex_offset -
                                                                    current_mesh->vertex_offset);
        current_mesh->index_count++;
    }
}

static void bake_gltf_mesh(
    arena_t*      scratch_arena,
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
        cgltf_primitive* primitive = &node->mesh->primitives[p];
        if (primitive->attributes_count == 0)
            continue;

        if (out_scene->mesh_count >= PAK_MAX_MESHES ||
            out_scene->vertex_count + primitive->attributes[0].data->count >= PAK_MAX_VERTICES) {
            log_error("Cooker out of memory! Too many meshes or vertices.");
            return;
        }

        pak_mesh_t* current_mesh            = &out_scene->meshes[out_scene->mesh_count++];
        current_mesh->model_id              = model_id;
        current_mesh->vertex_offset         = out_scene->vertex_count;
        current_mesh->index_offset          = out_scene->index_count;
        current_mesh->vertex_count          = primitive->attributes[0].data->count;
        current_mesh->index_count           = 0;
        current_mesh->base_color_texture_id = -1;
        current_mesh->normal_texture_id     = -1;
        current_mesh->ao_roughness_metallic_texture_id = -1;

        uint32_t prim_vertex_offset = out_scene->vertex_count;
        out_scene->vertex_count += current_mesh->vertex_count;

        vec4_t    base_color;
        cgltf_int uv_index;
        bool      has_normal_map;
        bool      is_alpha_masked;
        parse_primitive_material(
            scratch_arena,
            primitive,
            data,
            img_map,
            out_scene,
            current_mesh,
            &base_color,
            &uv_index,
            &has_normal_map,
            &is_alpha_masked
        );
        current_mesh->is_alpha_masked = is_alpha_masked;
        parse_primitive_vertices(
            primitive,
            out_scene,
            current_mesh,
            prim_vertex_offset,
            global_transform,
            base_color,
            uv_index
        );
        parse_primitive_indices(primitive, out_scene, current_mesh, prim_vertex_offset);

        if (has_normal_map) {
            uint32_t* temp_indices     = NULL;
            uint32_t  temp_index_count = 0;

            if (primitive->indices) {
                temp_index_count = primitive->indices->count;
                temp_indices     = malloc(temp_index_count * sizeof(uint32_t));
                for (size_t i = 0; i < temp_index_count; i++) {
                    temp_indices[i] = cgltf_accessor_read_index(primitive->indices, i);
                }
            }

            calculate_tangents(
                &out_scene->vertices[prim_vertex_offset],
                current_mesh->vertex_count,
                temp_indices,
                temp_index_count
            );

            if (temp_indices) {
                free(temp_indices);
            }
        }
    }
}

static void bake_gltf_node(
    arena_t*      scratch_arena,
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
    // mat4_t global_transform = mat4_mul(local_transform, parent_transform);
    mat4_t global_transform = mat4_mul(parent_transform, local_transform);
    bake_gltf_mesh(scratch_arena, node, global_transform, out_scene, model_id, data, img_map);
    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(
            scratch_arena, node->children[i], global_transform, out_scene, model_id, data, img_map
        );
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

static uint32_t calculate_bc7_mip_chain_size(uint32_t w, uint32_t h, uint32_t mip_levels) {
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < mip_levels; i++) {
        uint32_t blocks_x = (w + 3) / 4;
        uint32_t blocks_y = (h + 3) / 4;
        total_size += blocks_x * blocks_y * 16; // 16 bytes per BC7 block
        w = (w > 1) ? w / 2 : 1;
        h = (h > 1) ? h / 2 : 1;
    }
    return total_size;
}

static bool try_load_from_cache(
    const char*   cache_path,
    void*         src_data,
    uint32_t      compressed_size,
    scene_desc_t* out_scene,
    uint32_t      tex_idx
) {
    FILE* cache_file = fopen(cache_path, "rb");
    if (!cache_file) {
        return false;
    }

    fseek(cache_file, 0, SEEK_END);
    long cached_size = ftell(cache_file);
    fseek(cache_file, 0, SEEK_SET);

    int w;
    int h;
    int channels;
    image_info_from_memory(src_data, compressed_size, &w, &h, &channels);

    uint32_t max_dim    = (w > h) ? w : h;
    uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;

    pak_texture_t* tex = &out_scene->textures[tex_idx];
    tex->byte_size     = cached_size;
    tex->width         = w;
    tex->height        = h;
    tex->channels      = 4;
    tex->format        = PAK_TEX_FORMAT_BC7_UNORM;
    tex->byte_offset   = 0;
    tex->mip_levels    = mip_levels;

    out_scene->raw_texture_bytes[tex_idx] = malloc(cached_size);
    fread(out_scene->raw_texture_bytes[tex_idx], 1, cached_size, cache_file);
    fclose(cache_file);

    log_info("  -> [CACHE HIT] Loaded BC7 from %s", cache_path);
    return true;
}

static void build_and_cache_texture(
    const char*   cache_path,
    void*         src_data,
    uint32_t      compressed_size,
    scene_desc_t* out_scene,
    uint32_t      tex_idx
) {
    int      w, h, channels;
    uint8_t* raw_pixels = image_load_from_memory(src_data, compressed_size, &w, &h, &channels, 4);
    if (!raw_pixels) {
        log_error("Failed to decode raw image data for texture %d", tex_idx);
        return;
    }

    uint32_t max_dim    = (w > h) ? w : h;
    uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;
    uint32_t total_size = calculate_bc7_mip_chain_size(w, h, mip_levels);

    pak_texture_t* tex = &out_scene->textures[tex_idx];
    tex->byte_size     = total_size;
    tex->width         = w;
    tex->height        = h;
    tex->channels      = 4;
    tex->format        = PAK_TEX_FORMAT_BC7_UNORM;
    tex->byte_offset   = 0;
    tex->mip_levels    = mip_levels;

    out_scene->raw_texture_bytes[tex_idx] = malloc(total_size);
    uint8_t* bc7_dst                      = out_scene->raw_texture_bytes[tex_idx];

    bc7enc_compress_block_params pack_params;
    bc7enc_compress_block_params_init_gltf(&pack_params);
    bc7enc_compress_block_params_init_linear_weights_gltf(&pack_params);
    pack_params.m_max_partitions_mode = 0;
    pack_params.m_try_least_squares   = BC7ENC_FALSE;
    pack_params.m_use_mode5_for_alpha = 1;
    pack_params.m_use_mode7_for_alpha = 1;

    uint32_t dst_offset         = 0;
    uint32_t mip_w              = w;
    uint32_t mip_h              = h;
    uint8_t* current_mip_pixels = raw_pixels;

    for (uint32_t mip = 0; mip < mip_levels; mip++) {
        uint32_t blocks_x = (mip_w + 3) / 4;
        uint32_t blocks_y = (mip_h + 3) / 4;

        for (uint32_t by = 0; by < blocks_y; by++) {
            for (uint32_t bx = 0; bx < blocks_x; bx++) {
                uint32_t block_pixels[16] = {0};
                for (uint32_t py = 0; py < 4; py++) {
                    for (uint32_t px = 0; px < 4; px++) {
                        uint32_t global_x = bx * 4 + px;
                        uint32_t global_y = by * 4 + py;

                        global_x = (global_x < mip_w) ? global_x : mip_w - 1;
                        global_y = (global_y < mip_h) ? global_y : mip_h - 1;

                        uint32_t src_idx = (global_y * mip_w + global_x) * 4;
                        block_pixels[py * 4 + px] =
                            ((uint32_t)current_mip_pixels[src_idx + 0] << 0) |
                            ((uint32_t)current_mip_pixels[src_idx + 1] << 8) |
                            ((uint32_t)current_mip_pixels[src_idx + 2] << 16) |
                            ((uint32_t)current_mip_pixels[src_idx + 3] << 24);
                    }
                }
                bc7enc_compress_block(&bc7_dst[dst_offset], block_pixels, &pack_params);
                dst_offset += 16;
            }
        }

        if (mip < mip_levels - 1) {
            uint32_t next_w          = (mip_w > 1) ? mip_w / 2 : 1;
            uint32_t next_h          = (mip_h > 1) ? mip_h / 2 : 1;
            uint8_t* next_mip_pixels = malloc(next_w * next_h * 4);

            image_resize_uint8_linear(
                current_mip_pixels, mip_w, mip_h, 0, next_mip_pixels, next_w, next_h, 0, IMAGE_RGBA
            );

            if (current_mip_pixels != raw_pixels)
                free(current_mip_pixels);

            current_mip_pixels = next_mip_pixels;
            mip_w              = next_w;
            mip_h              = next_h;
        }
    }

    if (current_mip_pixels != raw_pixels)
        free(current_mip_pixels);
    image_free(raw_pixels);

    FILE* write_cache = fopen(cache_path, "wb");
    if (write_cache) {
        fwrite(bc7_dst, 1, total_size, write_cache);
        fclose(write_cache);
        log_info("  -> [CACHE MISS] Compressed %d Mips & Saved to %s", mip_levels, cache_path);
    } else {
        log_warn("  -> Failed to write cache. Does .cache/ exist?");
    }
}

static void process_texture_job(bake_context_t* ctx, uint32_t gltf_idx, uint32_t tex_idx) {
    if (!ctx->opt_fast_textures)
        return;

    cgltf_image* gltf_img = &ctx->data->images[gltf_idx];
    void* src_data = (uint8_t*)gltf_img->buffer_view->buffer->data + gltf_img->buffer_view->offset;
    uint32_t comp_size = gltf_img->buffer_view->size;

    uint64_t data_hash = hash_data(src_data, comp_size);
    char     cache_path[512];
    snprintf(
        cache_path, sizeof(cache_path), "./.cache/tex_%llx.bc7", (unsigned long long)data_hash
    );

    if (try_load_from_cache(cache_path, src_data, comp_size, ctx->out_scene, tex_idx)) {
        return;
    }
    build_and_cache_texture(cache_path, src_data, comp_size, ctx->out_scene, tex_idx);
}


int texture_worker_thread(void* user_data) {
    bake_context_t* ctx = (bake_context_t*)user_data;

    while (true) {
        // Safely grab the next image index to process
        int i = platform_atomic_int_add(&ctx->next_job_idx, 1);
        if (i >= ctx->total_images) {
            break; // No more images to process
        }

        int tex_idx = ctx->img_map[i];
        if (tex_idx == -1) {
            continue; // Skipped texture
        }

        cgltf_image* gltf_img = &ctx->data->images[i];

        // Ensure the image has a valid buffer view (embedded GLB texture)
        if (gltf_img->buffer_view && gltf_img->buffer_view->buffer->data) {

            // Calculate where this specific image's bytes start in the GLB file
            uint8_t* buffer_start    = (uint8_t*)gltf_img->buffer_view->buffer->data;
            uint8_t* image_data_ptr  = buffer_start + gltf_img->buffer_view->offset;
            size_t   image_data_size = gltf_img->buffer_view->size;

            int w, h, channels;

            // Decode the embedded PNG/JPG directly into raw RAM
            uint8_t* raw_pixels = image_load_from_memory(
                image_data_ptr, (uint32_t)image_data_size, &w, &h, &channels, 4
            );

            if (!raw_pixels) {
                log_error("Worker failed to decode raw image %d", i);
                continue;
            }

            // Assign to the parallel array
            ctx->out_scene->raw_texture_bytes[tex_idx] = raw_pixels;
    
            ctx->out_scene->textures[tex_idx].width      = w;
            ctx->out_scene->textures[tex_idx].height     = h;
            ctx->out_scene->textures[tex_idx].channels   = 4;
            ctx->out_scene->textures[tex_idx].byte_size  = w * h * 4;
            ctx->out_scene->textures[tex_idx].format     = PAK_TEX_FORMAT_RGBA8_UNORM;
            ctx->out_scene->textures[tex_idx].mip_levels = 1;
        }
    }

    return 0;
}

bool bake_model(
    arena_t*      scratch_arena,
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
        root_transform.m[1][1] = 0.0f;
        root_transform.m[1][2] = 1.0f;
        root_transform.m[2][1] = -1.0f;
        root_transform.m[2][2] = 0.0f;
    }

    for (size_t i = 0; i < gltf_scene->nodes_count; i++) {
        bake_gltf_node(
            scratch_arena, gltf_scene->nodes[i], root_transform, out_scene, model_id, data, img_map
        );
    }

    free(img_map);
    cgltf_free(data);
    return true;
}
