

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/resources/image.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/platform/platform.h"
#include "gltf.h"
#include "tools/core/mesh_utilities.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "bc7/bc7enc.h"

typedef struct {
    uint8_t* pixels;
    int      width;
    int      height;
    int      channels;
} raw_image_t;

typedef struct {
    scene_desc_t* out_scene;
    cgltf_data*   data;
    int32_t*      img_map;
    int32_t*      orm_map;
    uint32_t      total_images;
    bool          opt_fast_textures;

    platform_atomic_int_t next_job_idx;
} bake_context_t;

static uint64_t hash_data(const void* data, size_t length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t       hash  = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

static inline void bc7enc_compress_block_params_init_gltf(bc7enc_compress_block_params* p) {
    p->m_max_partitions_mode                  = BC7ENC_MAX_PARTITIONS1;
    p->m_try_least_squares                    = BC7ENC_TRUE;
    p->m_mode_partition_estimation_filterbank = BC7ENC_TRUE;
    p->m_uber_level                           = 0;
    p->m_use_mode5_for_alpha                  = BC7ENC_TRUE;
    p->m_use_mode7_for_alpha                  = BC7ENC_TRUE;
    p->m_perceptual                           = BC7ENC_TRUE;
    p->m_weights[0]                           = 128;
    p->m_weights[1]                           = 64;
    p->m_weights[2]                           = 16;
    p->m_weights[3]                           = 32;
}

static uint32_t calculate_mip_count(uint32_t w, uint32_t h) {
    uint32_t max_dim = (w > h) ? w : h;
    uint32_t levels  = 1;
    while (max_dim > 1) {
        max_dim /= 2;
        levels++;
    }
    return levels;
}

static uint32_t calculate_bc7_mip_chain_size(uint32_t w, uint32_t h, uint32_t mip_levels) {
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < mip_levels; i++) {
        uint32_t blocks_x = (w + 3) / 4;
        uint32_t blocks_y = (h + 3) / 4;
        total_size += blocks_x * blocks_y * 16;
        w = (w > 1) ? w / 2 : 1;
        h = (h > 1) ? h / 2 : 1;
    }
    return total_size;
}

static void compress_and_cache_raw_pixels(
    const char*   cache_path,
    uint8_t*      raw_pixels,
    int           w,
    int           h,
    scene_desc_t* out_scene,
    uint32_t      tex_idx
) {
    uint32_t max_dim = (w > h) ? w : h;
    // uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;
    uint32_t mip_levels = calculate_mip_count(w, h);
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
    pack_params.m_perceptual          = BC7ENC_FALSE;
    pack_params.m_weights[0]          = 1;
    pack_params.m_weights[1]          = 1;
    pack_params.m_weights[2]          = 1;
    pack_params.m_weights[3]          = 1;
    pack_params.m_max_partitions_mode = 0;
    pack_params.m_try_least_squares   = BC7ENC_FALSE;

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

    FILE* write_cache = platform_file_open_write(cache_path);
    if (write_cache) {
        platform_file_write(write_cache, bc7_dst, total_size);
        platform_file_close(write_cache);
        log_info("  -> [CACHE MISS] Compressed %d Mips & Saved to %s", mip_levels, cache_path);
    } else {
        log_warn("  -> Failed to write cache. Does .cache/ exist?");
    }
}

static bool try_load_from_cache_raw(
    const char*   cache_path,
    int           w,
    int           h,
    scene_desc_t* out_scene,
    uint32_t      tex_idx
) {
    FILE* cache_file = fopen(cache_path, "rb");
    if (!cache_file)
        return false;

    fseek(cache_file, 0, SEEK_END);
    long cached_size = ftell(cache_file);
    fseek(cache_file, 0, SEEK_SET);

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

bool gltf_measure_model(
    arena_t*              scratch,
    const char*           filepath,
    gltf_model_metrics_t* out_metrics,
    texture_registry_t*   texture_registry
) {
    memset(out_metrics, 0, sizeof(gltf_model_metrics_t));

    cgltf_options options = {0};
    cgltf_data*   data    = NULL;

    if (cgltf_parse_file(&options, filepath, &data) != cgltf_result_success) {
        log_error("gltf_measure_model: Failed to parse GLTF header for %s", filepath);
        return false;
    }

    out_metrics->material_count = (uint32_t)data->materials_count;

    for (size_t i = 0; i < data->images_count; i++) {
        cgltf_image* image = &data->images[i];

        if (image->uri) {
            if (texture_registry_add(scratch, texture_registry, image->uri)) {
                out_metrics->texture_count++;
            }
        } else {
            out_metrics->texture_count++;
        }
    }

    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material* mat = &data->materials[i];
        if (mat->has_pbr_metallic_roughness) {
            cgltf_texture_view* mr = &mat->pbr_metallic_roughness.metallic_roughness_texture;
            cgltf_texture_view* ao = &mat->occlusion_texture;

            int32_t mr_idx = (mr->texture && mr->texture->image)
                                 ? (int32_t)(mr->texture->image - data->images)
                                 : -1;
            int32_t ao_idx = (ao->texture && ao->texture->image)
                                 ? (int32_t)(ao->texture->image - data->images)
                                 : -1;

            if (mr_idx == -1 && ao_idx == -1) {
                // No maps
            } else if (mr_idx == ao_idx && mr_idx != -1) {
                // Already packed by artist
            } else {
                out_metrics->texture_count++;
            }
        }
    }

    for (size_t m = 0; m < data->meshes_count; m++) {
        cgltf_mesh* mesh = &data->meshes[m];
        out_metrics->mesh_count += mesh->primitives_count;

        for (size_t p = 0; p < mesh->primitives_count; p++) {
            cgltf_primitive* prim = &mesh->primitives[p];
            if (prim->indices) {
                out_metrics->index_count += prim->indices->count;
            }
            for (size_t a = 0; a < prim->attributes_count; a++) {
                if (prim->attributes[a].type == cgltf_attribute_type_position) {
                    out_metrics->vertex_count += prim->attributes[a].data->count;
                    break;
                }
            }
        }
    }

    cgltf_free(data);
    return true;
}

static int texture_worker_thread(void* user_data) {
    bake_context_t* ctx = (bake_context_t*)user_data;
    while (true) {
        int i = platform_atomic_int_add(&ctx->next_job_idx, 1);
        if (i >= ctx->total_images) {
            break; // No more images to process
        }

        int tex_idx = ctx->img_map[i];
        if (tex_idx == -1) {
            continue; // Skipped texture
        }

        cgltf_image* gltf_img = &ctx->data->images[i];

        if (gltf_img->buffer_view && gltf_img->buffer_view->buffer->data) {
            uint8_t* buffer_start    = (uint8_t*)gltf_img->buffer_view->buffer->data;
            uint8_t* image_data_ptr  = buffer_start + gltf_img->buffer_view->offset;
            size_t   image_data_size = gltf_img->buffer_view->size;

            if (ctx->opt_fast_textures) {
                int      w, h, channels;
                uint8_t* raw_pixels = image_load_from_memory(
                    image_data_ptr, (uint32_t)image_data_size, &w, &h, &channels, 4
                );
                if (!raw_pixels)
                    continue;

                ctx->out_scene->raw_texture_bytes[tex_idx]   = raw_pixels;
                ctx->out_scene->textures[tex_idx].width      = w;
                ctx->out_scene->textures[tex_idx].height     = h;
                ctx->out_scene->textures[tex_idx].channels   = 4;
                ctx->out_scene->textures[tex_idx].byte_size  = w * h * 4;
                ctx->out_scene->textures[tex_idx].format     = PAK_TEX_FORMAT_RGBA8_UNORM;
                ctx->out_scene->textures[tex_idx].mip_levels = 1;
            } else {
                uint64_t data_hash = hash_data(image_data_ptr, image_data_size);
                char     cache_path[512];
                snprintf(
                    cache_path,
                    sizeof(cache_path),
                    "./.cache/tex_%llx.bc7",
                    (unsigned long long)data_hash
                );

                int w, h, channels;
                image_info_from_memory(
                    image_data_ptr, (uint32_t)image_data_size, &w, &h, &channels
                );

                if (!try_load_from_cache_raw(cache_path, w, h, ctx->out_scene, tex_idx)) {
                    uint8_t* raw = image_load_from_memory(
                        image_data_ptr, (uint32_t)image_data_size, &w, &h, &channels, 4
                    );
                    if (raw) {
                        compress_and_cache_raw_pixels(
                            cache_path, raw, w, h, ctx->out_scene, tex_idx
                        );
                        image_free(raw);
                    }
                }
            }
        } else if (gltf_img->uri) {
            log_warn(
                "Texture [%d] uses external URI '%s'. The cooker currently only supports embedded "
                "GLB textures!",
                i,
                gltf_img->uri
            );
        }
    }

    return 0;
}

/**
 * Creates an ambient-occlusion, roughness, and metallic packed map.
 */
static raw_image_t pack_orm_texture(
    raw_image_t* ambient_occlusion_map,
    raw_image_t* metallic_roughness_map
) {
    int width  = 1;
    int height = 1;
    if (metallic_roughness_map && metallic_roughness_map->width > width) {
        width = metallic_roughness_map->width;
    }
    if (ambient_occlusion_map && ambient_occlusion_map->width > width) {
        width = ambient_occlusion_map->width;
    }
    if (metallic_roughness_map && metallic_roughness_map->height > height) {
        height = metallic_roughness_map->height;
    }
    if (ambient_occlusion_map && ambient_occlusion_map->height > height) {
        height = ambient_occlusion_map->height;
    }

    raw_image_t packed;
    packed.width    = width;
    packed.height   = height;
    packed.channels = 4;
    packed.pixels   = malloc(width * height * 4);

    if (!packed.pixels) {
        return packed;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (float)x / (float)width;
            float v = (float)y / (float)height;

            uint8_t ambient_occlusion = 255;
            if (ambient_occlusion_map && ambient_occlusion_map->pixels) {
                int src_x   = (int)(u * ambient_occlusion_map->width);
                int src_y   = (int)(v * ambient_occlusion_map->height);
                int src_idx = (src_y * ambient_occlusion_map->width + src_x) *
                              ambient_occlusion_map->channels;
                ambient_occlusion = ambient_occlusion_map->pixels[src_idx + 0];
            }

            uint8_t roughness = 255;
            uint8_t metallic  = 0;
            if (metallic_roughness_map && metallic_roughness_map->pixels) {
                int src_x   = (int)(u * metallic_roughness_map->width);
                int src_y   = (int)(v * metallic_roughness_map->height);
                int src_idx = (src_y * metallic_roughness_map->width + src_x) *
                              metallic_roughness_map->channels;
                if (metallic_roughness_map->channels >= 3) {
                    roughness = metallic_roughness_map->pixels[src_idx + 1];
                    metallic  = metallic_roughness_map->pixels[src_idx + 2];
                }
            }
            int out_idx = (y * width + x) * 4;

            packed.pixels[out_idx + 0] = ambient_occlusion;
            packed.pixels[out_idx + 1] = roughness;
            packed.pixels[out_idx + 2] = metallic;
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
    int32_t*         orm_map,
    scene_desc_t*    out_scene,
    pak_mesh_t*      current_mesh,
    uint32_t         model_id,
    bool             opt_fast_textures,
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
    log_info("primitive naterial: '%s', alpha mode: %s", mat_name, alpha_mode_str);

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

        log_debug(
            "pbr: metallic: %.2f, roughness: %.2f",
            current_mesh->metallic_factor,
            current_mesh->roughness_factor
        );

        cgltf_texture_view* base_color_view =
            &primitive->material->pbr_metallic_roughness.base_color_texture;

        if (base_color_view->texture && base_color_view->texture->image) {
            *out_uv_index                       = base_color_view->texcoord;
            size_t img_idx                      = base_color_view->texture->image - data->images;
            current_mesh->base_color_texture_id = img_map[img_idx];

            if (current_mesh->base_color_texture_id != -1) {
                pak_texture_format_t* fmt =
                    &out_scene->textures[current_mesh->base_color_texture_id].format;
                if (*fmt == PAK_TEX_FORMAT_PNG_UNORM)
                    *fmt = PAK_TEX_FORMAT_PNG_SRGB;
                else if (*fmt == PAK_TEX_FORMAT_RGBA8_UNORM)
                    *fmt = PAK_TEX_FORMAT_RGBA8_SRGB;
                else if (*fmt == PAK_TEX_FORMAT_BC7_UNORM)
                    *fmt = PAK_TEX_FORMAT_BC7_SRGB;
            }
            log_debug("base color texture: pak id %d", current_mesh->base_color_texture_id);
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

        if (mr_id == -1 && ao_id == -1) {
            current_mesh->ao_roughness_metallic_texture_id = -1;
            log_debug("material '%s' has no mr or ao maps.", mat_name);

        } else if (mr_id == ao_id && mr_id != -1) {
            current_mesh->ao_roughness_metallic_texture_id = mr_id;
            log_debug("orm map: pak id %d (Already Packed)", mr_id);

        } else {

            size_t mat_idx = primitive->material - data->materials;

            if (orm_map[mat_idx] != -1) {
                current_mesh->ao_roughness_metallic_texture_id = orm_map[mat_idx];
                log_info("cache hit: re-using orm map: pak id %d", orm_map[mat_idx]);
            } else {
                uint32_t new_tex_idx = out_scene->texture_count++;

                int final_w = 1;
                int final_h = 1;
                if (mr_id != -1 && out_scene->textures[mr_id].width > final_w)
                    final_w = out_scene->textures[mr_id].width;
                if (ao_id != -1 && out_scene->textures[ao_id].width > final_w)
                    final_w = out_scene->textures[ao_id].width;
                if (mr_id != -1 && out_scene->textures[mr_id].height > final_h)
                    final_h = out_scene->textures[mr_id].height;
                if (ao_id != -1 && out_scene->textures[ao_id].height > final_h)
                    final_h = out_scene->textures[ao_id].height;

                if (opt_fast_textures) {
                    raw_image_t ao_raw = {0};
                    if (ao_id != -1) {
                        ao_raw.width    = out_scene->textures[ao_id].width;
                        ao_raw.height   = out_scene->textures[ao_id].height;
                        ao_raw.channels = 4;
                        ao_raw.pixels   = out_scene->raw_texture_bytes[ao_id];
                    }
                    raw_image_t mr_raw = {0};
                    if (mr_id != -1) {
                        mr_raw.width    = out_scene->textures[ao_id].width;
                        mr_raw.height   = out_scene->textures[ao_id].height;
                        mr_raw.channels = 4;
                        mr_raw.pixels   = out_scene->raw_texture_bytes[mr_id];
                    }

                    raw_image_t packed = pack_orm_texture(
                        (ao_id != -1) ? &ao_raw : NULL, (mr_id != -1) ? &mr_raw : NULL
                    );

                    out_scene->textures[new_tex_idx].width     = packed.width;
                    out_scene->textures[new_tex_idx].height    = packed.height;
                    out_scene->textures[new_tex_idx].channels  = packed.channels;
                    out_scene->textures[new_tex_idx].byte_size = packed.width * packed.height *
                                                                 packed.channels;
                    out_scene->textures[new_tex_idx].mip_levels = 1;
                    out_scene->textures[new_tex_idx].format     = PAK_TEX_FORMAT_RGBA8_UNORM;
                    out_scene->raw_texture_bytes[new_tex_idx]   = packed.pixels;
                } else {
                    char cache_path[512];
                    snprintf(
                        cache_path,
                        sizeof(cache_path),
                        "./.cache/orm_mod%u_mat%zu.bc7",
                        model_id,
                        mat_idx
                    );

                    if (!try_load_from_cache_raw(
                            cache_path, final_w, final_h, out_scene, new_tex_idx
                        )) {
                        log_info("  - Generating ORM Map for cache...");

                        cgltf_image* ao_img = (ao_view->texture) ? ao_view->texture->image : NULL;
                        cgltf_image* mr_img = (mr_view->texture) ? mr_view->texture->image : NULL;

                        int      ao_w  = 0;
                        int      ao_h  = 0;
                        int      ao_c  = 0;
                        uint8_t* ao_px = NULL;
                        if (ao_img && ao_img->buffer_view) {
                            uint8_t* ptr = (uint8_t*)ao_img->buffer_view->buffer->data +
                                           ao_img->buffer_view->offset;
                            ao_px = image_load_from_memory(
                                ptr, ao_img->buffer_view->size, &ao_w, &ao_h, &ao_c, 4
                            );
                        }

                        int      mr_w  = 0;
                        int      mr_h  = 0;
                        int      mr_c  = 0;
                        uint8_t* mr_px = NULL;
                        if (mr_img && mr_img->buffer_view) {
                            uint8_t* ptr = (uint8_t*)mr_img->buffer_view->buffer->data +
                                           mr_img->buffer_view->offset;
                            mr_px = image_load_from_memory(
                                ptr, mr_img->buffer_view->size, &mr_w, &mr_h, &mr_c, 4
                            );
                        }

                        raw_image_t ao_raw = {0};
                        if (ao_px) {
                            ao_raw.width    = ao_w;
                            ao_raw.height   = ao_h;
                            ao_raw.channels = 4;
                            ao_raw.pixels   = ao_px;
                        }
                        raw_image_t mr_raw = {0};
                        if (mr_px) {
                            mr_raw.width    = mr_w;
                            mr_raw.height   = mr_h;
                            mr_raw.channels = 4;
                            mr_raw.pixels   = mr_px;
                        }

                        raw_image_t packed = pack_orm_texture(
                            (ao_px) ? &ao_raw : NULL, (mr_px) ? &mr_raw : NULL
                        );

                        compress_and_cache_raw_pixels(
                            cache_path,
                            packed.pixels,
                            packed.width,
                            packed.height,
                            out_scene,
                            new_tex_idx
                        );

                        free(packed.pixels);
                        if (ao_px)
                            image_free(ao_px);
                        if (mr_px)
                            image_free(mr_px);
                    }
                }

                current_mesh->ao_roughness_metallic_texture_id = new_tex_idx;
                orm_map[mat_idx]                               = new_tex_idx;

                log_debug("  - Synthesized ORM Map -> PAK ID %d", new_tex_idx);
            }
        }
    } else {
        // Fallback for meshes that completely lack a PBR material definition
        current_mesh->metallic_factor  = 0.0f; // Default to non-metal
        current_mesh->roughness_factor = 1.0f; // Default to fully rough (matte)
        log_debug("  ! WARNING: Material '%s' is not PBR. Using fallback factors.", mat_name);
    }

    cgltf_texture* nrm_tex = primitive->material->normal_texture.texture;
    if (nrm_tex && nrm_tex->image) {
        size_t img_idx                  = nrm_tex->image - data->images;
        current_mesh->normal_texture_id = img_map[img_idx];
        *out_has_normal_map             = true;
        log_debug(
            "  - Normal Map Texture -> PAK ID %d (Left as UNORM)", current_mesh->normal_texture_id
        );
    } else {
        log_debug("  ! WARNING: Material '%s' has NO Normal Map. Using flat fallback.", mat_name);
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
    int32_t*      img_map,
    int32_t*      orm_map,
    bool          opt_fast_textures
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
            orm_map,
            out_scene,
            current_mesh,
            model_id,
            opt_fast_textures,
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
    int32_t*      img_map,
    int32_t*      orm_map,
    bool          opt_fast_textures
) {
    mat4_t local_transform = mat4_identity();
    if (node->has_matrix || node->has_translation || node->has_rotation || node->has_scale) {
        cgltf_float matrix[16];
        cgltf_node_transform_local(node, matrix);
        memcpy(&local_transform, matrix, sizeof(mat4_t));
    }
    // mat4_t global_transform = mat4_mul(local_transform, parent_transform);
    mat4_t global_transform = mat4_mul(parent_transform, local_transform);
    bake_gltf_mesh(
        scratch_arena,
        node,
        global_transform,
        out_scene,
        model_id,
        data,
        img_map,
        orm_map,
        opt_fast_textures
    );
    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(
            scratch_arena,
            node->children[i],
            global_transform,
            out_scene,
            model_id,
            data,
            img_map,
            orm_map,
            opt_fast_textures
        );
    }
}

#define MAX_WORKER_THREADS 64

bool gltf_bake_model(
    arena_t*      scratch_arena,
    const char*   filepath,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    bool          opt_fast_textures,
    bool          opt_z_up
) {
    cgltf_options options = {0};
    cgltf_data*   data    = NULL;

    if (cgltf_parse_file(&options, filepath, &data) != cgltf_result_success) {
        log_error("CGLTF Failed to parse model: %s", filepath);
        return false;
    }

    uint32_t exact_vertices = 0;
    uint32_t exact_indices  = 0;
    uint32_t exact_meshes   = 0;

    for (size_t m = 0; m < data->meshes_count; m++) {
        cgltf_mesh* mesh = &data->meshes[m];
        exact_meshes += mesh->primitives_count;

        for (size_t p = 0; p < mesh->primitives_count; p++) {
            cgltf_primitive* prim = &mesh->primitives[p];
            if (prim->indices) {
                exact_indices += prim->indices->count;
            }
            for (size_t a = 0; a < prim->attributes_count; a++) {
                if (prim->attributes[a].type == cgltf_attribute_type_position) {
                    exact_vertices += prim->attributes[a].data->count;
                    break;
                }
            }
        }
    }

    out_scene->vertices = arena_push_array(scratch_arena, pak_vertex_t, exact_vertices);
    out_scene->indices  = arena_push_array(scratch_arena, uint32_t, exact_indices);
    out_scene->meshes   = arena_push_array(scratch_arena, pak_mesh_t, exact_meshes);

    uint32_t max_possible_textures = data->images_count + data->materials_count;
    out_scene->textures = arena_push_array(scratch_arena, pak_texture_t, max_possible_textures);
    out_scene->raw_texture_bytes = arena_push_array(scratch_arena, uint8_t*, max_possible_textures);

    cgltf_load_buffers(&options, data, filepath);

    int32_t* img_map = malloc(data->images_count * sizeof(int32_t));
    for (size_t i = 0; i < data->images_count; i++) {
        cgltf_image* gltf_img = &data->images[i];
        if (gltf_img->buffer_view != NULL && out_scene->texture_count < PAK_MAX_TEXTURES) {
            img_map[i] = out_scene->texture_count++;
            log_debug("GLTF Image [%zu] -> Mapped to PAK Texture ID: %d", i, img_map[i]);
        } else {
            img_map[i] = -1;
        }
    }

    int32_t* orm_map = malloc(data->materials_count * sizeof(int32_t));
    for (size_t i = 0; i < data->materials_count; i++) {
        orm_map[i] = -1;
    }

    bake_context_t ctx = {
        .out_scene         = out_scene,
        .data              = data,
        .img_map           = img_map,
        .orm_map           = orm_map,
        .total_images      = data->images_count,
        .opt_fast_textures = opt_fast_textures
    };
    platform_atomic_int_set(&ctx.next_job_idx, 0);

    int num_cores = platform_get_core_count();
    if (num_cores > MAX_WORKER_THREADS)
        num_cores = MAX_WORKER_THREADS;
    platform_thread_t threads[MAX_WORKER_THREADS];

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
            log_error("CGLTF: No scenes found in %s", filepath);
            cgltf_free(data);
            free(img_map);
            free(orm_map);
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
            scratch_arena,
            gltf_scene->nodes[i],
            root_transform,
            out_scene,
            model_id,
            data,
            img_map,
            orm_map,
            opt_fast_textures
        );
    }

    free(img_map);
    free(orm_map);
    cgltf_free(data);
    return true;
}
