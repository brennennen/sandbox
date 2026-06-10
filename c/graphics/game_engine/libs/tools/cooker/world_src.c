
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        if (!pak_vertices) {
            log_error("CRITICAL: Out of memory in pak_arena while packing vertices!");
            return false;
        }
        out_pak_header->vertex_offset = arena_get_offset_of(pak_arena, pak_vertices);
        out_pak_header->vertex_count  = staged_scene->vertex_count;
        memcpy(pak_vertices, staged_scene->vertices, vertex_size);
    }

    if (staged_scene->index_count > 0) {
        size_t index_size  = staged_scene->index_count * sizeof(uint32_t);
        void*  pak_indices = arena_push_aligned(pak_arena, index_size, 16);
        if (!pak_indices) {
            log_error("CRITICAL: Out of memory in pak_arena while packing indices!");
            return false;
        }
        out_pak_header->index_offset = arena_get_offset_of(pak_arena, pak_indices);
        out_pak_header->index_count  = staged_scene->index_count;
        memcpy(pak_indices, staged_scene->indices, index_size);
    }

    if (staged_scene->mesh_count > 0) {
        size_t mesh_size  = staged_scene->mesh_count * sizeof(pak_mesh_t);
        void*  pak_meshes = arena_push_aligned(pak_arena, mesh_size, 16);
        if (!pak_meshes) {
            log_error("CRITICAL: Out of memory in pak_arena while packing meshes!");
            return false;
        }
        out_pak_header->mesh_offset = arena_get_offset_of(pak_arena, pak_meshes);
        out_pak_header->mesh_count  = staged_scene->mesh_count;
        memcpy(pak_meshes, staged_scene->meshes, mesh_size);
    }

    if (staged_scene->entity_count > 0) {
        size_t entity_size  = staged_scene->entity_count * sizeof(pak_entity_t);
        void*  pak_entities = arena_push_aligned(pak_arena, entity_size, 16);
        if (!pak_entities) {
            log_error("CRITICAL: Out of memory in pak_arena while packing entities!");
            return false;
        }
        out_pak_header->entity_offset = arena_get_offset_of(pak_arena, pak_entities);
        out_pak_header->entity_count  = staged_scene->entity_count;
        memcpy(pak_entities, staged_scene->entities, entity_size);
    }

    if (staged_scene->texture_count > 0) {
        size_t         tex_headers_size = staged_scene->texture_count * sizeof(pak_texture_t);
        pak_texture_t* pak_textures     = arena_push_aligned(pak_arena, tex_headers_size, 16);
        if (!pak_textures) {
            log_error("CRITICAL: Out of memory in pak_arena while packing texture headers!");
            return false;
        }

        out_pak_header->texture_offset = arena_get_offset_of(pak_arena, pak_textures);
        out_pak_header->texture_count  = staged_scene->texture_count;
        memcpy(pak_textures, staged_scene->textures, tex_headers_size);

        for (uint32_t i = 0; i < staged_scene->texture_count; i++) {
            pak_texture_t* tex = &pak_textures[i];

            if (tex->byte_size > 0 && staged_scene->raw_texture_bytes[i] != NULL) {
                void* tex_payload = arena_push_aligned(pak_arena, tex->byte_size, 16);
                if (!tex_payload) {
                    log_error(
                        "CRITICAL: Out of memory in pak_arena while packing texture payload %d!", i
                    );
                    return false;
                }

                tex->byte_offset = arena_get_offset_of(pak_arena, tex_payload);
                memcpy(tex_payload, staged_scene->raw_texture_bytes[i], tex->byte_size);

                free(staged_scene->raw_texture_bytes[i]);
                staged_scene->raw_texture_bytes[i] = NULL;
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

bool process_world_source(
    arena_t*      pak_arena,
    arena_t*      scratch_arena,
    world_src_t*  world_source,
    string_span_t base_dir_span,
    world_pak_t*  out_world_pak
) {
    world_pak_t* pak_header              = arena_push_struct(pak_arena, world_pak_t);
    pak_header->magic                    = PAK_MAGIC;
    pak_header->version                  = 1;
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
            return false;
        }
    }

    log_info("Initializing Staging Scene...");
    scene_desc_t* staged_scene  = arena_push_struct(scratch_arena, scene_desc_t);
    staged_scene->vertex_count  = 0;
    staged_scene->index_count   = 0;
    staged_scene->mesh_count    = 0;
    staged_scene->texture_count = 0;
    staged_scene->entity_count  = 0;

    for (uint32_t i = 0; i < world_source->layer_count; i++) {
        layer_src_t* layer      = &world_source->layers[i];
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
                    model_path,
                    staged_scene,
                    curr_model->id,
                    curr_model->fast_textures,
                    curr_model->z_up
                )) {
                log_error("Failed to bake model %u", curr_model->id);
                return false;
            }
            curr_model = curr_model->next;
        }

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
    }

    if (!pack_scene_to_arena(pak_arena, staged_scene, pak_header)) {
        log_error("Failed to pack scene into final arena!");
        return false;
    }

    log_info("World processing complete!");
    return true;
}

bool write_world_pak(arena_t* pak_arena, const char* output_file) {
    log_info("Writing PAK file to %s...", output_file);

    FILE* out_file = fopen(output_file, "wb");
    if (!out_file) {
        log_error("Failed to open output file: %s", output_file);
        return false;
    }

    size_t written = fwrite(pak_arena->buffer, 1, pak_arena->offset, out_file);
    fclose(out_file);

    if (written != pak_arena->offset) {
        log_error("Failed to write full PAK payload. Disk full?");
        return false;
    }

    log_info("Successfully wrote %zu bytes to %s", written, output_file);
    return true;
}
