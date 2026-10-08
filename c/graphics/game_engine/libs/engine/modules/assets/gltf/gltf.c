

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/resources/image.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/platform/platform.h"
#include "gltf.h"
#include "tools/core/mesh_utilities.h"

#include "core/resources/compressed_texture.h"

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

typedef struct {
    vec4_t    base_color;
    cgltf_int uv_index;
    bool      has_normal_map;
    bool      is_alpha_masked;
} material_properties_t;

static uint64_t hash_data(const void* data, size_t length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t       hash  = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

static void compress_and_cache_raw_pixels(
    const char*   cache_path,
    uint8_t*      raw_pixels,
    int           w,
    int           h,
    scene_desc_t* out_scene,
    uint32_t      tex_idx
) {
    compressed_texture_t compressed = {0};

    if (!texture_compress_bc7(&compressed, raw_pixels, w, h, false)) {
        log_error("Failed to compress texture %d", tex_idx);
        return;
    }

    pak_texture_t* tex = &out_scene->textures[tex_idx];
    tex->byte_size     = compressed.total_size;
    tex->width         = w;
    tex->height        = h;
    tex->channels      = 4;
    tex->format        = PAK_TEX_FORMAT_BC7_UNORM;
    tex->byte_offset   = 0;
    tex->mip_levels    = compressed.mip_levels;

    out_scene->raw_texture_bytes[tex_idx] = compressed.compressed_bytes;

    FILE* write_cache = fopen(cache_path, "wb");
    if (write_cache) {
        fwrite(compressed.compressed_bytes, 1, compressed.total_size, write_cache);
        fclose(write_cache);
        log_info(
            "  -> [CACHE MISS] Compressed %d Mips & Saved to %s", compressed.mip_levels, cache_path
        );
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

    uint32_t max_dim = (w > h) ? w : h;
    // uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;
    uint32_t mip_levels = calculate_mip_count(w, h);

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
    packed.pixels   = calloc(width * height, 4);

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

static void parse_base_properties(
    cgltf_material*        mat,
    bake_context_t*        bake_context,
    pak_mesh_t*            current_mesh,
    material_properties_t* out_material_properties
) {
    if (mat->alpha_mode == cgltf_alpha_mode_mask || mat->alpha_mode == cgltf_alpha_mode_blend) {
        out_material_properties->is_alpha_masked = true;
    }

    if (mat->has_pbr_metallic_roughness) {
        cgltf_pbr_metallic_roughness* pbr   = &mat->pbr_metallic_roughness;
        out_material_properties->base_color = (vec4_t){pbr->base_color_factor[0],
                                                       pbr->base_color_factor[1],
                                                       pbr->base_color_factor[2],
                                                       pbr->base_color_factor[3]};

        current_mesh->metallic_factor  = pbr->metallic_factor;
        current_mesh->roughness_factor = pbr->roughness_factor;

        cgltf_texture_view* base_view = &pbr->base_color_texture;
        if (base_view->texture && base_view->texture->image) {
            out_material_properties->uv_index = base_view->texcoord;
            current_mesh->base_color_texture_id =
                bake_context->img_map[base_view->texture->image - bake_context->data->images];

            // Upgrade the format to SRGB for the base color map
            if (current_mesh->base_color_texture_id != -1) {
                pak_texture_format_t* fmt =
                    &bake_context->out_scene->textures[current_mesh->base_color_texture_id].format;
                if (*fmt == PAK_TEX_FORMAT_PNG_UNORM)
                    *fmt = PAK_TEX_FORMAT_PNG_SRGB;
                if (*fmt == PAK_TEX_FORMAT_RGBA8_UNORM)
                    *fmt = PAK_TEX_FORMAT_RGBA8_SRGB;
                if (*fmt == PAK_TEX_FORMAT_BC7_UNORM)
                    *fmt = PAK_TEX_FORMAT_BC7_SRGB;
            }
        }
    } else {
        current_mesh->metallic_factor  = 0.0f;
        current_mesh->roughness_factor = 1.0f;
    }
}

static void parse_normal_map(
    bake_context_t* bake_context,
    cgltf_material* mat,
    pak_mesh_t*     current_mesh,
    bool*           out_has_normal_map
) {
    cgltf_texture* nrm_tex = mat->normal_texture.texture;
    if (nrm_tex && nrm_tex->image) {
        current_mesh->normal_texture_id =
            bake_context->img_map[nrm_tex->image - bake_context->data->images];
        *out_has_normal_map = true;
    }
}

static void synthesize_fast_orm_map(
    int32_t       ao_id,
    int32_t       mr_id,
    scene_desc_t* out_scene,
    uint32_t      new_tex_idx
) {
    raw_image_t ao_raw = {0};
    if (ao_id != -1) {
        ao_raw.width    = out_scene->textures[ao_id].width;
        ao_raw.height   = out_scene->textures[ao_id].height;
        ao_raw.channels = 4;
        ao_raw.pixels   = out_scene->raw_texture_bytes[ao_id];
    }

    raw_image_t mr_raw = {0};
    if (mr_id != -1) {
        mr_raw.width    = out_scene->textures[mr_id].width;
        mr_raw.height   = out_scene->textures[mr_id].height;
        mr_raw.channels = 4;
        mr_raw.pixels   = out_scene->raw_texture_bytes[mr_id];
    }

    raw_image_t packed = pack_orm_texture(
        (ao_id != -1) ? &ao_raw : NULL, (mr_id != -1) ? &mr_raw : NULL
    );

    pak_texture_t* tex                        = &out_scene->textures[new_tex_idx];
    tex->width                                = packed.width;
    tex->height                               = packed.height;
    tex->channels                             = packed.channels;
    tex->byte_size                            = packed.width * packed.height * packed.channels;
    tex->mip_levels                           = 1;
    tex->format                               = PAK_TEX_FORMAT_RGBA8_UNORM;
    out_scene->raw_texture_bytes[new_tex_idx] = packed.pixels;
}

static void synthesize_cached_orm_map(
    cgltf_texture_view* ao_view,
    cgltf_texture_view* mr_view,
    scene_desc_t*       out_scene,
    uint32_t            new_tex_idx,
    int                 final_w,
    int                 final_h
) {
    // Generate a hash from the raw embedded pixel bytes to prevent stale caches
    uint64_t orm_hash = 0x0123456789ABCDEFULL;
    if (ao_view->texture && ao_view->texture->image && ao_view->texture->image->buffer_view) {
        cgltf_image* img = ao_view->texture->image;
        uint8_t*     ptr = (uint8_t*)img->buffer_view->buffer->data + img->buffer_view->offset;
        orm_hash ^= hash_data(ptr, img->buffer_view->size);
    }
    if (mr_view->texture && mr_view->texture->image && mr_view->texture->image->buffer_view) {
        cgltf_image* img = mr_view->texture->image;
        uint8_t*     ptr = (uint8_t*)img->buffer_view->buffer->data + img->buffer_view->offset;
        orm_hash ^= hash_data(ptr, img->buffer_view->size);
    }

    char cache_path[512];
    snprintf(cache_path, sizeof(cache_path), "./.cache/orm_%llx.bc7", (unsigned long long)orm_hash);

    if (!try_load_from_cache_raw(cache_path, final_w, final_h, out_scene, new_tex_idx)) {
        log_info("  - Generating ORM Map for cache...");

        int      ao_w = 0, ao_h = 0, ao_c = 0;
        uint8_t* ao_px = NULL;
        if (ao_view->texture && ao_view->texture->image && ao_view->texture->image->buffer_view) {
            cgltf_image* img = ao_view->texture->image;
            uint8_t*     ptr = (uint8_t*)img->buffer_view->buffer->data + img->buffer_view->offset;
            ao_px = image_load_from_memory(ptr, img->buffer_view->size, &ao_w, &ao_h, &ao_c, 4);
        }

        int      mr_w = 0, mr_h = 0, mr_c = 0;
        uint8_t* mr_px = NULL;
        if (mr_view->texture && mr_view->texture->image && mr_view->texture->image->buffer_view) {
            cgltf_image* img = mr_view->texture->image;
            uint8_t*     ptr = (uint8_t*)img->buffer_view->buffer->data + img->buffer_view->offset;
            mr_px = image_load_from_memory(ptr, img->buffer_view->size, &mr_w, &mr_h, &mr_c, 4);
        }

        raw_image_t ao_raw = {.width = ao_w, .height = ao_h, .channels = 4, .pixels = ao_px};
        raw_image_t mr_raw = {.width = mr_w, .height = mr_h, .channels = 4, .pixels = mr_px};

        raw_image_t packed = pack_orm_texture((ao_px) ? &ao_raw : NULL, (mr_px) ? &mr_raw : NULL);

        compress_and_cache_raw_pixels(
            cache_path, packed.pixels, packed.width, packed.height, out_scene, new_tex_idx
        );

        free(packed.pixels);
        if (ao_px)
            image_free(ao_px);
        if (mr_px)
            image_free(mr_px);
    }
}

static void parse_orm_maps(
    bake_context_t* bake_contet,
    cgltf_material* mat,
    scene_desc_t*   out_scene,
    pak_mesh_t*     current_mesh,
    bool            opt_fast_textures
) {
    cgltf_texture_view* mr_view = &mat->pbr_metallic_roughness.metallic_roughness_texture;
    cgltf_texture_view* ao_view = &mat->occlusion_texture;

    int32_t mr_id = (mr_view->texture && mr_view->texture->image)
                        ? bake_contet->img_map[mr_view->texture->image - bake_contet->data->images]
                        : -1;
    int32_t ao_id = (ao_view->texture && ao_view->texture->image)
                        ? bake_contet->img_map[ao_view->texture->image - bake_contet->data->images]
                        : -1;

    if (mr_id == -1 && ao_id == -1) {
        current_mesh->ao_roughness_metallic_texture_id = -1;
        return; // Nothing to do
    }

    if (mr_id == ao_id && mr_id != -1) {
        current_mesh->ao_roughness_metallic_texture_id = mr_id;
        return; // Artist already packed it
    }

    size_t mat_idx = mat - bake_contet->data->materials;
    if (bake_contet->orm_map[mat_idx] != -1) {
        current_mesh->ao_roughness_metallic_texture_id = bake_contet->orm_map[mat_idx];
        return; // Already synthesized in a previous mesh
    }

    // Synthesis Required
    uint32_t new_tex_idx = out_scene->texture_count++;

    int final_w = 1, final_h = 1;
    if (mr_id != -1) {
        if (out_scene->textures[mr_id].width > final_w)
            final_w = out_scene->textures[mr_id].width;
        if (out_scene->textures[mr_id].height > final_h)
            final_h = out_scene->textures[mr_id].height;
    }
    if (ao_id != -1) {
        if (out_scene->textures[ao_id].width > final_w)
            final_w = out_scene->textures[ao_id].width;
        if (out_scene->textures[ao_id].height > final_h)
            final_h = out_scene->textures[ao_id].height;
    }

    if (opt_fast_textures)
        synthesize_fast_orm_map(ao_id, mr_id, out_scene, new_tex_idx);
    else
        synthesize_cached_orm_map(ao_view, mr_view, out_scene, new_tex_idx, final_w, final_h);

    current_mesh->ao_roughness_metallic_texture_id = new_tex_idx;
    bake_contet->orm_map[mat_idx]                  = new_tex_idx;
}

static void parse_primitive_material(
    bake_context_t*        bake_context,
    cgltf_primitive*       primitive,
    scene_desc_t*          out_scene,
    pak_mesh_t*            current_mesh,
    bool                   opt_fast_textures,
    material_properties_t* out_material_properties
) {
    out_material_properties->base_color            = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};
    out_material_properties->uv_index              = 0;
    out_material_properties->has_normal_map        = false;
    out_material_properties->is_alpha_masked       = true;
    current_mesh->base_color_texture_id            = -1;
    current_mesh->normal_texture_id                = -1;
    current_mesh->ao_roughness_metallic_texture_id = -1;

    if (!primitive->material) {
        log_info("Primitive has NO Material attached.");
        return;
    }

    parse_base_properties(primitive->material, bake_context, current_mesh, out_material_properties);

    if (primitive->material->has_pbr_metallic_roughness) {
        parse_orm_maps(
            bake_context, primitive->material, out_scene, current_mesh, opt_fast_textures
        );
    }

    parse_normal_map(
        bake_context, primitive->material, current_mesh, &out_material_properties->has_normal_map
    );
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
    if (!primitive->indices) {
        return;
    }

    for (size_t i = 0; i < primitive->indices->count; i++) {
        uint32_t raw_index = cgltf_accessor_read_index(primitive->indices, i);
        out_scene->indices[out_scene->index_count++] = raw_index + (prim_vertex_offset -
                                                                    current_mesh->vertex_offset);
        current_mesh->index_count++;
    }
}

static void bake_gltf_mesh(
    bake_context_t* bake_context,
    arena_t*        scratch_arena,
    cgltf_node*     node,
    mat4_t          global_transform,
    scene_desc_t*   out_scene,
    uint32_t        model_id,
    bool            opt_fast_textures
) {
    if (!node->mesh) {
        return;
    }

    for (size_t p = 0; p < node->mesh->primitives_count; p++) {
        cgltf_primitive* primitive = &node->mesh->primitives[p];
        if (primitive->attributes_count == 0) {
            continue;
        }

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

        material_properties_t material_properties = {0};
        parse_primitive_material(
            bake_context,
            primitive,
            out_scene,
            current_mesh,
            opt_fast_textures,
            &material_properties
        );
        current_mesh->is_alpha_masked = material_properties.is_alpha_masked;
        parse_primitive_vertices(
            primitive,
            out_scene,
            current_mesh,
            prim_vertex_offset,
            global_transform,
            material_properties.base_color,
            material_properties.uv_index
        );
        parse_primitive_indices(primitive, out_scene, current_mesh, prim_vertex_offset);

        if (material_properties.has_normal_map) {
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
    bake_context_t* bake_context,
    arena_t*        scratch_arena,
    cgltf_node*     node,
    mat4_t          parent_transform,
    scene_desc_t*   out_scene,
    uint32_t        model_id,
    bool            opt_fast_textures
) {
    mat4_t local_transform = mat4_identity();
    if (node->has_matrix || node->has_translation || node->has_rotation || node->has_scale) {
        cgltf_float matrix[16];
        cgltf_node_transform_local(node, matrix);
        memcpy(&local_transform, matrix, sizeof(mat4_t));
    }

    mat4_t global_transform = mat4_mul(parent_transform, local_transform);
    bake_gltf_mesh(
        bake_context,
        scratch_arena,
        node,
        global_transform,
        out_scene,
        model_id,
        opt_fast_textures
    );

    for (size_t i = 0; i < node->children_count; i++) {
        bake_gltf_node(
            bake_context,
            scratch_arena,
            node->children[i],
            global_transform,
            out_scene,
            model_id,
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
            &ctx,
            scratch_arena,
            gltf_scene->nodes[i],
            root_transform,
            out_scene,
            model_id,
            opt_fast_textures
        );
    }

    free(img_map);
    free(orm_map);
    cgltf_free(data);
    return true;
}
