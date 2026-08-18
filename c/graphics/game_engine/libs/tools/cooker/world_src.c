
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/platform/platform.h"

#include "core/resources/compressed_texture.h"

#include "engine/core/logger.h"

#include "libs/core/resources/image.h"
#include "libs/tools/cooker/parsers/common_source_parser.h"
#include "libs/tools/parsers/parser_source_types.h"

#include "libs/tools/cooker/parsers/layer_source_parser.h"
#include "libs/tools/cooker/world_src.h"

#include "ibl_processor.h"

#include "libs/tools/bakers/gltf_baker.h"

#include "engine/core/math/mat4.h"

static bool parse_environment_statement(
    arena_t*           arena,
    string_span_t*     cursor,
    string_span_t      base_dir,
    environment_src_t* out_environment_src
) {
    log_info("parse_environment_statement");
    while (true) {
        token_t next = get_next_token(cursor);

        if (next.type == TOK_EOF) {
            break;
        }
        if (next.type == TOK_ERROR) {
            int err_len = next.length;
            if (next.length > 10) {
                err_len = 10;
            }
            log_error("LEXER ERROR: Unexpected character near '%.*s'", err_len, next.start);
            return false;
        }
        if (next.type == TOK_IDENTIFIER) {
            string_span_t token_span = span_init(next.start, next.length);
            if (span_equals_cstr(token_span, "Skybox")) {
                token_t skybox_tok = get_next_token(cursor);
                int     len        = skybox_tok.length < 255 ? skybox_tok.length : 255;
                out_environment_src->skybox_path = span_init(skybox_tok.start, skybox_tok.length);
                log_info(
                    "Skybox: '%.*s'",
                    (int)out_environment_src->skybox_path.length,
                    out_environment_src->skybox_path.data
                );
            } else if (span_equals_cstr(token_span, "AmbientTint")) {
                token_t r = get_next_token(cursor);
                token_t g = get_next_token(cursor);
                token_t b = get_next_token(cursor);

                out_environment_src->ambient_tint.x = r.float_value;
                out_environment_src->ambient_tint.y = g.float_value;
                out_environment_src->ambient_tint.z = b.float_value;
                log_info("AmbientTint: %f, %f, %f", r.float_value, g.float_value, b.float_value);
            } else if (span_equals_cstr(token_span, "FogDensity")) {
                token_t fog_tok                  = get_next_token(cursor);
                out_environment_src->fog_density = fog_tok.float_value;
                log_info("FogDensity: %f", fog_tok.float_value);
            }
        } else if (next.type == TOK_RBRACE) {
            log_info("parse_environment_statement: end");
            break;
        }
    }
    return true;
}

static bool pack_scene_to_arena(
    arena_t*      pak_arena,
    scene_desc_t* staged_scene,
    world_pak_t*  out_pak_header
) {
    log_info("Packing staged scene into contiguous arena...");

    if (staged_scene->vertex_count > 0) {
        size_t vertex_size  = staged_scene->vertex_count * sizeof(pak_vertex_t);
        void*  pak_vertices = arena_push_aligned(pak_arena, vertex_size, 16);
        if (!pak_vertices)
            return false;
        out_pak_header->vertex_offset = arena_get_offset_of(pak_arena, pak_vertices);
        out_pak_header->vertex_count  = staged_scene->vertex_count;
        memcpy(pak_vertices, staged_scene->vertices, vertex_size);
    }

    if (staged_scene->index_count > 0) {
        size_t index_size  = staged_scene->index_count * sizeof(uint32_t);
        void*  pak_indices = arena_push_aligned(pak_arena, index_size, 16);
        if (!pak_indices)
            return false;
        out_pak_header->index_offset = arena_get_offset_of(pak_arena, pak_indices);
        out_pak_header->index_count  = staged_scene->index_count;
        memcpy(pak_indices, staged_scene->indices, index_size);
    }

    if (staged_scene->mesh_count > 0) {
        size_t mesh_size  = staged_scene->mesh_count * sizeof(pak_mesh_t);
        void*  pak_meshes = arena_push_aligned(pak_arena, mesh_size, 16);
        if (!pak_meshes)
            return false;
        out_pak_header->mesh_offset = arena_get_offset_of(pak_arena, pak_meshes);
        out_pak_header->mesh_count  = staged_scene->mesh_count;
        memcpy(pak_meshes, staged_scene->meshes, mesh_size);
    }

    if (staged_scene->entity_count > 0) {
        size_t entity_size  = staged_scene->entity_count * sizeof(pak_entity_t);
        void*  pak_entities = arena_push_aligned(pak_arena, entity_size, 16);
        if (!pak_entities)
            return false;
        out_pak_header->entity_offset = arena_get_offset_of(pak_arena, pak_entities);
        out_pak_header->entity_count  = staged_scene->entity_count;
        memcpy(pak_entities, staged_scene->entities, entity_size);
    }

    if (staged_scene->texture_count > 0) {
        size_t         tex_headers_size = staged_scene->texture_count * sizeof(pak_texture_t);
        pak_texture_t* pak_textures     = arena_push_aligned(pak_arena, tex_headers_size, 16);
        if (!pak_textures)
            return false;

        out_pak_header->texture_offset = arena_get_offset_of(pak_arena, pak_textures);
        out_pak_header->texture_count  = staged_scene->texture_count;
        memcpy(pak_textures, staged_scene->textures, tex_headers_size);

        uint64_t current_virtual_offset = pak_arena->offset;
        for (uint32_t i = 0; i < staged_scene->texture_count; i++) {
            pak_texture_t* tex = &pak_textures[i];

            if (tex->byte_size > 0 && staged_scene->raw_texture_bytes[i] != NULL) {
                tex->byte_offset = current_virtual_offset;
                current_virtual_offset += tex->byte_size;
            }
        }
    }

    return true;
}

bool parse_world_source(
    arena_t*      arena,
    string_span_t text,
    string_span_t base_dir,
    world_src_t*  out_world_src
) {
    log_info("parse_world_source");

    string_span_t cursor = text;

    while (true) {
        token_t next = get_next_token(&cursor);
        if (next.type == TOK_EOF) {
            break;
        }
        if (next.type == TOK_ERROR) {
            int err_len = next.length;
            if (next.length > 10) {
                err_len = 10;
            }
            log_error("LEXER ERROR: Unexpected character near '%.*s'", err_len, next.start);
            return false;
        }
        if (next.type == TOK_IDENTIFIER) {
            string_span_t token_span = span_init(next.start, next.length);
            if (span_equals_cstr(token_span, "World")) {
                log_info("Parsing World block...");
                token_t name_tok    = get_next_token(&cursor);
                out_world_src->name = span_init(name_tok.start, name_tok.length);
                log_info(
                    "  -> name: '%.*s'", (int)out_world_src->name.length, out_world_src->name.data
                );
                token_t brace_tok = get_next_token(&cursor); // {
                continue;
            } else if (span_equals_cstr(token_span, "Version")) {
                token_t version_tok = get_next_token(&cursor);
                memcpy(out_world_src->version, version_tok.start, version_tok.length);
                log_info("  -> version: %f", version_tok.float_value);
            } else if (span_equals_cstr(token_span, "Environment")) {
                token_t env_name_tok            = get_next_token(&cursor);
                token_t env_brace_tok           = get_next_token(&cursor);
                out_world_src->environment.name = span_init(
                    env_name_tok.start, env_name_tok.length
                );
                log_info(
                    "  -> environment: '%.*s'",
                    out_world_src->environment.name.length,
                    out_world_src->environment.name.data
                );
                parse_environment_statement(arena, &cursor, base_dir, &out_world_src->environment);
            } else if (span_equals_cstr(token_span, "Include")) {
                log_info("Include");
                token_t       path_tok  = get_next_token(&cursor);
                string_span_t path_span = span_init(path_tok.start, path_tok.length);
                log_info("  -> Path: %.*s", (int)path_span.length, path_span.data);

                if (out_world_src->layer_count == 0) {
                    out_world_src->layers = (layer_src_t*)(arena->buffer + arena->offset);
                }
                layer_src_t* layer = arena_push_struct(arena, layer_src_t);
                out_world_src->layer_count++;
                layer->path = path_span;

                size_t full_path_len   = base_dir.length + 1 + path_span.length;
                char*  full_layer_path = (char*)arena_push(arena, full_path_len + 1);
                memcpy(full_layer_path, base_dir.data, base_dir.length);
                full_layer_path[base_dir.length] = '/';
                memcpy(full_layer_path + base_dir.length + 1, path_span.data, path_span.length);
                full_layer_path[full_path_len] = '\0';
                string_span_t layer_text;
                if (arena_read_file_to_span(arena, full_layer_path, &layer_text)) {
                    if (!parse_layer_source(arena, layer_text, base_dir, layer)) {
                        log_error("Failed to parse included layer: %s", full_layer_path);
                        return false;
                    }
                } else {
                    log_error("Failed to read included layer file: %s", full_layer_path);
                    return false;
                }
            } else {
                log_info("unrecognized top-level keyword: '%.*s'", (int)next.length, next.start);
            }
        } else if (next.type == TOK_RBRACE) { // }
            log_info("End of block");
        }
    }
    return true;
}

static void print_environment_statement(const environment_src_t* environment_src) {
    log_info("env:");
    log_info(
        "skybox_path: '%.*s'",
        environment_src->skybox_path.length,
        environment_src->skybox_path.data
    );
}

void print_world_source(const world_src_t* world_src) {
    log_info("world_src:");
    log_info("World: '%.*s'", (int)world_src->name.length, world_src->name.data);
    log_info("Version: %s", world_src->version);
    print_environment_statement(&world_src->environment);
}

static bool process_environment(
    arena_t*           pak_arena,
    arena_t*           scratch_arena,
    environment_src_t* env_src,
    string_span_t      base_dir,
    environment_pak_t* out_pak_env
) {
    log_info("Processing environment...");
    out_pak_env->ambient_tint = env_src->ambient_tint;
    out_pak_env->fog_density  = env_src->fog_density;

    if (env_src->skybox_path.length == 0) {
        log_warn("No skybox path provided.");
        return true;
    }

    char* skypath_path_cstr = span_to_arena_cstr(env_src->skybox_path, scratch_arena);
    char  full_path_cstr[1024];
    snprintf(
        full_path_cstr,
        sizeof(full_path_cstr),
        "%.*s/%s",
        (int)base_dir.length,
        base_dir.data,
        skypath_path_cstr
    );
    log_info("  -> Loading Skybox: %s", full_path_cstr);
    string_span_t full_path = span_from_cstr(full_path_cstr);

    int width;
    int height;
    int channels;

    float* hdr_pixels = image_loadf(pak_arena, full_path, &width, &height, &channels, 4);
    if (!hdr_pixels) {
        log_error("Failed to load HDR skybox: %s", image_failure_reason());
        return false;
    }

    size_t payload_size                     = (size_t)width * (size_t)height * 4 * sizeof(float);
    out_pak_env->skybox_cubemap.width       = (uint32_t)width;
    out_pak_env->skybox_cubemap.height      = (uint32_t)height;
    out_pak_env->skybox_cubemap.channels    = 4;
    out_pak_env->skybox_cubemap.format      = PAK_TEX_FORMAT_RGBA32F;
    out_pak_env->skybox_cubemap.data_size   = (uint64_t)payload_size;
    out_pak_env->skybox_cubemap.data_offset = pak_arena->offset;

    void* pak_pixel_dest = arena_push_aligned(pak_arena, payload_size, 16);
    memcpy(pak_pixel_dest, hdr_pixels, payload_size);

    image_free(hdr_pixels);

    log_info(
        "  -> Skybox Packed: %dx%d (%.2f MB) at offset %llu",
        width,
        height,
        (float)payload_size / (1024 * 1024),
        (unsigned long long)out_pak_env->skybox_cubemap.data_offset
    );

    return true;
}

static bool process_layer_entities(scene_desc_t* staged_scene, layer_src_t* layer) {
    entity_src_t* curr_entity = layer->entities;

    while (curr_entity != NULL) {
        if (staged_scene->entity_count >= PAK_MAX_ENTITIES) {
            log_error("Exceeded PAK_MAX_ENTITIES!");
            return false;
        }

        pak_entity_t* out_entity = &staged_scene->entities[staged_scene->entity_count++];
        out_entity->model_id     = curr_entity->model_id;

        float rx = curr_entity->rotation.x * (M_PI / 180.0f);
        float ry = curr_entity->rotation.y * (M_PI / 180.0f);
        float rz = curr_entity->rotation.z * (M_PI / 180.0f);

        mat4_t t_mat = mat4_translate(curr_entity->position);
        mat4_t s_mat = mat4_scale(curr_entity->scale);
        mat4_t rot_x = mat4_rotate_x(rx);
        mat4_t rot_y = mat4_rotate_y(ry);
        mat4_t rot_z = mat4_rotate_z(rz);

        mat4_t r_mat          = mat4_mul(mat4_mul(rot_z, rot_x), rot_y);
        mat4_t sr_mat         = mat4_mul(r_mat, s_mat);
        out_entity->transform = mat4_mul(t_mat, sr_mat);

        log_info(
            "Processed Entity: %.*s (Model %u)",
            (int)curr_entity->name.length,
            curr_entity->name.data,
            curr_entity->model_id
        );

        curr_entity = curr_entity->next;
    }
    return true;
}

static uint64_t hash_data(const void* data, size_t length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t       hash  = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

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

static bool compress_staged_textures(arena_t* scratch_arena, scene_desc_t* scene) {
    log_info("--- PHASE 3: COMPRESSING TEXTURES ---");

    // Make sure the cache directory actually exists!
    platform_create_directory(span_from_cstr(".cache"));

    for (uint32_t i = 0; i < scene->texture_count; i++) {
        pak_texture_t* tex_header      = &scene->textures[i];
        uint8_t*       raw_rgba_pixels = scene->raw_texture_bytes[i];

        if (!raw_rgba_pixels || tex_header->byte_size == 0) {
            continue; // Skip empty textures
        }

        // Only compress if it's currently flagged as a raw UNORM/SRGB format
        if (tex_header->format == PAK_TEX_FORMAT_BC7_UNORM ||
            tex_header->format == PAK_TEX_FORMAT_BC7_SRGB) {
            continue; // Already processed
        }

        uint64_t data_hash = hash_data(raw_rgba_pixels, tex_header->byte_size);
        // data_hash ^= (uint64_t)target_format * 0x100000001B3ULL;
        // data_hash ^= (uint64_t)tex_header->width * 0x100000001B3ULL;

        char cache_path[512];
        snprintf(
            cache_path, sizeof(cache_path), "./.cache/tex_%llx.bc7", (unsigned long long)data_hash
        );

        // Determine target format (preserve colorspace)
        pak_texture_format_t target_format = PAK_TEX_FORMAT_BC7_UNORM;
        bool                 is_srgb       = false;
        if (tex_header->format == PAK_TEX_FORMAT_RGBA8_SRGB ||
            tex_header->format == PAK_TEX_FORMAT_PNG_SRGB) {
            target_format = PAK_TEX_FORMAT_BC7_SRGB;
            is_srgb       = true;
        }

        FILE* cache_file = fopen(cache_path, "rb");
        if (cache_file) {
            // Re-calculate the expected size/mips so we know how much arena to request
            uint32_t w          = tex_header->width;
            uint32_t h          = tex_header->height;
            uint32_t max_dim    = (w > h) ? w : h;
            uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;

            fseek(cache_file, 0, SEEK_END);
            size_t cached_size = ftell(cache_file);
            fseek(cache_file, 0, SEEK_SET);

            // uint8_t* bc7_buffer = arena_push(scratch_arena, cached_size);
            // fread(bc7_buffer, 1, cached_size, cache_file);
            // fclose(cache_file);
            uint8_t* bc7_buffer = malloc(cached_size);
            fread(bc7_buffer, 1, cached_size, cache_file);
            fclose(cache_file);
            free(raw_rgba_pixels);

            scene->raw_texture_bytes[i] = bc7_buffer;
            tex_header->byte_size       = cached_size;
            tex_header->format          = target_format;
            tex_header->mip_levels      = mip_levels;

            log_info(
                "   -> [%d/%d] [CACHE HIT] Loaded %s", i + 1, scene->texture_count, cache_path
            );
        } else {
            compressed_texture_t compressed = {0};
            bool                 success    = texture_compress_bc7(
                &compressed,
                scratch_arena,
                raw_rgba_pixels,
                tex_header->width,
                tex_header->height,
                is_srgb
            );

            if (!success) {
                log_error(
                    "Failed to compress texture %d! (Width: %d, Height: %d)",
                    i,
                    tex_header->width,
                    tex_header->height
                );
                continue;
            }

            FILE* write_cache = fopen(cache_path, "wb");
            if (write_cache) {
                fwrite(compressed.compressed_bytes, 1, compressed.total_size, write_cache);
                fclose(write_cache);
            } else {
                log_warn("   -> [WARNING] Failed to write cache file: %s", cache_path);
            }

            scene->raw_texture_bytes[i] = compressed.compressed_bytes;
            tex_header->byte_size       = compressed.total_size;
            tex_header->format          = target_format;
            tex_header->mip_levels      = compressed.mip_levels;

            log_info(
                "   -> [%d/%d] [CACHE MISS] Compressed & Cached %s",
                i + 1,
                scene->texture_count,
                cache_path
            );
        }
    }
    return true;
}

static void deduplicate_textures(scene_desc_t* scene) {
    log_info("--- PHASE 2.5: DEDUPLICATING TEXTURES ---");

    uint32_t* remap_table = malloc(scene->texture_count * sizeof(uint32_t));
    uint64_t* hashes      = malloc(scene->texture_count * sizeof(uint64_t));

    uint32_t unique_count = 0;

    for (uint32_t i = 0; i < scene->texture_count; i++) {
        pak_texture_t* tex        = &scene->textures[i];
        uint8_t*       raw_pixels = scene->raw_texture_bytes[i];

        // Generate a rigorous hash for the image
        uint64_t h = 0;
        if (tex->byte_size > 0 && raw_pixels != NULL) {
            h = hash_data(raw_pixels, tex->byte_size);
            h ^= (uint64_t)tex->format * 0x100000001B3ULL;
            h ^= (uint64_t)tex->width * 0x100000001B3ULL;
        } else {
            // Unique hash for empty/fallback texture slots so they don't incorrectly merge
            h = 0xDEADBEEF00000000ULL | i;
        }

        // Search for an existing identical texture
        int found_idx = -1;
        for (uint32_t j = 0; j < unique_count; j++) {
            if (hashes[j] == h) {
                found_idx = j;
                break;
            }
        }

        if (found_idx != -1) {
            // DUPLICATE FOUND: Discard it!
            remap_table[i] = found_idx;

            if (raw_pixels) {
                free(raw_pixels);
                scene->raw_texture_bytes[i] = NULL;
            }
            tex->byte_size = 0;

        } else {
            // UNIQUE TEXTURE: Keep it and compact the array
            remap_table[i]       = unique_count;
            hashes[unique_count] = h;

            if (i != unique_count) {
                scene->textures[unique_count]          = scene->textures[i];
                scene->raw_texture_bytes[unique_count] = scene->raw_texture_bytes[i];

                scene->raw_texture_bytes[i]  = NULL;
                scene->textures[i].byte_size = 0;
            }
            unique_count++;
        }
    }

    log_info(
        "Deduplication Complete: %u total textures -> %u unique textures",
        scene->texture_count,
        unique_count
    );

    // Remap all meshes to point to the new compacted texture indices
    for (uint32_t i = 0; i < scene->mesh_count; i++) {
        pak_mesh_t* m = &scene->meshes[i];

        // Assuming your IDs are signed (e.g. -1 means no texture)
        if (m->base_color_texture_id >= 0 && m->base_color_texture_id < scene->texture_count)
            m->base_color_texture_id = remap_table[m->base_color_texture_id];

        if (m->normal_texture_id >= 0 && m->normal_texture_id < scene->texture_count)
            m->normal_texture_id = remap_table[m->normal_texture_id];

        if (m->ao_roughness_metallic_texture_id >= 0 &&
            m->ao_roughness_metallic_texture_id < scene->texture_count)
            m->ao_roughness_metallic_texture_id = remap_table[m->ao_roughness_metallic_texture_id];
    }

    scene->texture_count = unique_count;

    free(remap_table);
    free(hashes);
}

scene_desc_t* process_world_source(
    arena_t*      pak_arena,
    arena_t*      scratch_arena,
    world_src_t*  world_source,
    string_span_t base_dir_span,
    world_pak_t*  out_world_pak
) {
    world_pak_t* pak_header = arena_push_struct(pak_arena, world_pak_t);
    pak_header->magic       = PAK_MAGIC;
    pak_header->version     = 1;
    // NOTE: Hardcode scene_type here or pull it from world_source if implemented
    pak_header->scene_type               = 1; // 1 = PAK_SCENE_TYPE_STATIC_LEVEL
    pak_header->environment.ambient_tint = world_source->environment.ambient_tint;
    pak_header->environment.fog_density  = world_source->environment.fog_density;

    char base_dir_cstr[512];
    snprintf(
        base_dir_cstr, sizeof(base_dir_cstr), "%.*s", (int)base_dir_span.length, base_dir_span.data
    );

    if (world_source->environment.skybox_path.length > 0) {
        char hdri_full_path[1024];
        snprintf(
            hdri_full_path,
            sizeof(hdri_full_path),
            "%s/%.*s",
            base_dir_cstr,
            (int)world_source->environment.skybox_path.length,
            world_source->environment.skybox_path.data
        );

        if (!process_ibl_textures(
                pak_arena, scratch_arena, hdri_full_path, &pak_header->environment
            )) {
            log_error("Failed to process HDRI");
            return NULL;
        }
    }

    log_info("Initializing Staging Scene...");
    scene_desc_t* staged_scene  = arena_push_struct(scratch_arena, scene_desc_t);
    staged_scene->vertex_count  = 0;
    staged_scene->index_count   = 0;
    staged_scene->mesh_count    = 0;
    staged_scene->texture_count = 0;
    staged_scene->entity_count  = 0;

    log_info("loading models");
    for (uint32_t i = 0; i < world_source->layer_count; i++) {
        layer_src_t* layer = &world_source->layers[i];

        // Process Models (bake_model MUST only load raw PNGs and synthesize ORM, no compression!)
        model_src_t* curr_model = layer->models;
        while (curr_model != NULL) {
            char model_path[1024];
            snprintf(
                model_path,
                sizeof(model_path),
                "%s/%.*s",
                base_dir_cstr,
                (int)curr_model->path.length,
                curr_model->path.data
            );

            log_info("Baking Model %u: %s", curr_model->id, model_path);

            if (!bake_model(
                    scratch_arena,
                    model_path,
                    staged_scene,
                    curr_model->id,
                    curr_model->fast_textures,
                    curr_model->z_up
                )) {
                log_error("Failed to bake model %u", curr_model->id);
                return NULL;
            }
            curr_model = curr_model->next;
        }

        if (!process_layer_entities(staged_scene, layer)) {
            return NULL;
        }
    }

    deduplicate_textures(staged_scene);

    if (!compress_staged_textures(scratch_arena, staged_scene)) {
        log_error("Failed to compress staged textures!");
        return NULL;
    }

    if (!pack_scene_to_arena(pak_arena, staged_scene, pak_header)) {
        log_error("Failed to pack scene into final arena!");
        return NULL;
    }

    log_info("World processing complete!");
    return staged_scene;
}

bool write_world_pak(arena_t* pak_arena, scene_desc_t* staged_scene, const char* output_file) {
    log_info("Writing streamable PAK file to %s...", output_file);

    platform_file_t out_file = platform_file_open_write(output_file);
    if (!out_file) {
        log_error("Failed to open output file: %s", output_file);
        return false;
    }

    // Write the TOC (Table of Contents).
    uint64_t written = platform_file_write(out_file, pak_arena->buffer, pak_arena->offset);
    if (written != pak_arena->offset) {
        log_error("Failed to write PAK TOC. Disk full?");
        platform_file_close(out_file);
        return false;
    }

    // Stream heavy payloads & record exact physical offsets using the safe platform API
    for (uint32_t i = 0; i < staged_scene->texture_count; i++) {
        uint8_t* payload = staged_scene->raw_texture_bytes[i];
        if (payload) {
            // Safely align the physical file cursor to 16 bytes
            uint64_t current_pos = platform_file_tell(out_file);
            int      padding     = (16 - (current_pos % 16)) % 16;

            if (padding > 0) {
                uint8_t zero_pad[16] = {0};
                platform_file_write(out_file, zero_pad, padding);
            }

            // Record the TRUE absolute stream offset
            staged_scene->textures[i].byte_offset = platform_file_tell(out_file);

            // Write payload and free heap memory immediately
            platform_file_write(out_file, payload, staged_scene->textures[i].byte_size);

            free(payload);
            staged_scene->raw_texture_bytes[i] = NULL;
        }
    }

    // Patch the TOC with the true offsets
    world_pak_t* header = (world_pak_t*)pak_arena->buffer;

    // Seek back to where the texture array started in the file
    platform_file_seek(out_file, header->texture_offset);

    // Overwrite the empty 0 offsets with the real payload addresses
    platform_file_write(
        out_file, staged_scene->textures, sizeof(pak_texture_t) * staged_scene->texture_count
    );

    platform_file_close(out_file);
    log_info("Successfully streamed world to %s!", output_file);
    return true;
}
