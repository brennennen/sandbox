#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/core/path.h"
#include "shared/math_types.h"

#include "engine/core/math/mat4.h"

#include "cooker.h"
#include "engine/core/logger.h"
#include "engine/modules/assets/gltf/gltf.h"
#include "engine/platform/platform.h"
#include "tools/cooker/world_src.h"
#include "tools/parsers/parser_source_types.h"

#include "tools/cooker/ibl_processor.h"

/**
 * Tracks the pak serialization while streaming multiple gltf models and
 * other entities into a single .pak file.
 */
typedef struct {
    platform_file_t out_file;

    // Running totals
    uint32_t total_vertices;
    uint32_t total_indices;
    uint32_t total_textures;

    // Entity file write heads
    uint64_t vertices_offset;
    uint64_t indices_offset;
    uint64_t meshes_offset;
    uint64_t textures_offset;
    uint64_t payloads_offset;
    uint64_t entities_offset;
} pak_serializing_state_t;

#define ALIGN_UP(value, alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))

static void calculate_pak_layout(scene_manifest_t* manifest) {
    // 16-byte alignment is a standard safe boundary for GPU/SIMD data structures
    uint64_t current_offset = ALIGN_UP(sizeof(world_pak_t), 16);

    manifest->offset_entities = current_offset;
    current_offset += sizeof(pak_section_header_t) +
                      manifest->total_entities * sizeof(pak_entity_t);
    current_offset = ALIGN_UP(current_offset, 16);

    manifest->offset_meshes = current_offset;
    current_offset += sizeof(pak_section_header_t) + manifest->total_meshes * sizeof(pak_mesh_t);
    current_offset = ALIGN_UP(current_offset, 16);

    manifest->offset_vertices = current_offset;
    current_offset += sizeof(pak_section_header_t) +
                      manifest->total_vertices * sizeof(pak_vertex_t);
    current_offset = ALIGN_UP(current_offset, 16);

    manifest->offset_indices = current_offset;
    current_offset += sizeof(pak_section_header_t) + manifest->total_indices * sizeof(uint32_t);
    current_offset = ALIGN_UP(current_offset, 16);

    manifest->offset_textures = current_offset;
    current_offset += sizeof(pak_section_header_t) +
                      manifest->total_textures * sizeof(pak_texture_t);
    current_offset = ALIGN_UP(current_offset, 16);

    manifest->offset_texture_payloads = current_offset;
}

bool pass_1_measure_scene(
    arena_t*          scratch,
    world_src_t*      world,
    string_span_t     base_dir,
    scene_manifest_t* manifest
) {
    log_debug("%s", __func__);
    arena_temp_t scratch_mark = arena_begin_temp(scratch);

    char base_dir_cstr[512];
    snprintf(base_dir_cstr, sizeof(base_dir_cstr), "%.*s", (int)base_dir.length, base_dir.data);

    texture_registry_t texture_registry = {0};
    texture_registry.capacity           = 512;
    texture_registry.paths = arena_push_array(scratch, const char*, texture_registry.capacity);

    for (uint32_t i = 0; i < world->layer_count; i++) {
        layer_src_t* layer = &world->layers[i];
        manifest->total_entities += layer->entity_count;

        for (model_src_t* curr_model = layer->models; curr_model != NULL;
             curr_model              = curr_model->next) {
            char model_path[1024];
            snprintf(
                model_path,
                sizeof(model_path),
                "%s/%.*s",
                base_dir_cstr,
                (int)curr_model->path.length,
                curr_model->path.data
            );

            gltf_model_metrics_t metrics = {0};
            if (!gltf_measure_model(scratch, model_path, &metrics, &texture_registry)) {
                log_error("Pass 1: Failed to measure model %s", model_path);
                arena_end_temp(scratch, scratch_mark);
                return false;
            }

            manifest->total_vertices += metrics.vertex_count;
            manifest->total_indices += metrics.index_count;
            manifest->total_meshes += metrics.mesh_count;
            manifest->total_textures += metrics.texture_count;
        }
    }

    // Delegate the byte-math and memory alignment
    calculate_pak_layout(manifest);

    log_info("Pass 1 Results:");
    log_info("  -> Total Vertices: %u", manifest->total_vertices);
    log_info("  -> Total Indices:  %u", manifest->total_indices);
    log_info("  -> Total Meshes:   %u", manifest->total_meshes);
    log_info("  -> Total Entities: %u", manifest->total_entities);
    log_info("  -> Total Textures: %u", manifest->total_textures);

    // Free the texture registry memory before returning
    arena_end_temp(scratch, scratch_mark);

    return true;
}

bool pass_1_measure_scene2(
    arena_t*          scratch,
    world_src_t*      world,
    string_span_t     base_dir,
    scene_manifest_t* manifest
) {
    log_info("%s", __func__);
    char base_dir_cstr[512];
    snprintf(base_dir_cstr, sizeof(base_dir_cstr), "%.*s", (int)base_dir.length, base_dir.data);

    texture_registry_t texture_registry = {0};
    texture_registry.capacity           = 512;
    texture_registry.paths = arena_push_array(scratch, const char*, texture_registry.capacity);

    for (uint32_t i = 0; i < world->layer_count; i++) {
        layer_src_t* layer = &world->layers[i];
        manifest->total_entities += layer->entity_count;
        for (model_src_t* curr_model = layer->models; curr_model != NULL;
             curr_model              = curr_model->next) {
            char model_path[1024];
            snprintf(
                model_path,
                sizeof(model_path),
                "%s/%.*s",
                base_dir_cstr,
                (int)curr_model->path.length,
                curr_model->path.data
            );
            gltf_model_metrics_t metrics = {0};
            if (!gltf_measure_model(scratch, model_path, &metrics, &texture_registry)) {
                log_error("Pass 1: Failed to measure model %s", model_path);
                return false;
            }

            manifest->total_vertices += metrics.vertex_count;
            manifest->total_indices += metrics.index_count;
            manifest->total_meshes += metrics.mesh_count;
            manifest->total_textures += metrics.texture_count;
        }
    }

    uint64_t current_offset = sizeof(world_pak_t);

    manifest->offset_entities = current_offset;
    current_offset += manifest->total_entities * sizeof(pak_entity_t);

    manifest->offset_meshes = current_offset;
    current_offset += manifest->total_meshes * sizeof(pak_mesh_t);

    manifest->offset_vertices = current_offset;
    current_offset += manifest->total_vertices * sizeof(pak_vertex_t);

    manifest->offset_indices = current_offset;
    current_offset += manifest->total_indices * sizeof(uint32_t);

    manifest->offset_textures = current_offset;
    current_offset += manifest->total_textures * sizeof(pak_texture_t);

    manifest->offset_texture_payloads = current_offset;

    log_info("Pass 1 Results:");
    log_info("  -> Total Vertices: %u", manifest->total_vertices);
    log_info("  -> Total Indices:  %u", manifest->total_indices);
    log_info("  -> Total Meshes:   %u", manifest->total_meshes);
    log_info("  -> Total Entities: %u", manifest->total_entities);
    log_info("  -> Total Textures: %u", manifest->total_textures);
    return true;
}

static void stream_env_map(pak_serializing_state_t* state, arena_t* arena, texture_pak_t* map) {
    if (map->data_size > 0) {
        void* pixels     = arena->buffer + map->data_offset;
        map->data_offset = state->payloads_offset;

        platform_file_seek(state->out_file, state->payloads_offset);
        platform_file_write(state->out_file, pixels, map->data_size);
        state->payloads_offset += map->data_size;
    }
}

static bool process_and_stream_environment(
    arena_t*                 model_arena,
    world_src_t*             world,
    world_pak_t*             world_pak,
    const char*              base_dir_cstr,
    pak_serializing_state_t* state
) {
    if (world->environment.skybox_path.length == 0) {
        return true;
    }

    log_info("Processing HDRI Environment Maps...");
    char hdri_full_path[1024];
    snprintf(
        hdri_full_path,
        sizeof(hdri_full_path),
        "%s/%.*s",
        base_dir_cstr,
        (int)world->environment.skybox_path.length,
        world->environment.skybox_path.data
    );

    arena_temp_t ibl_mark = arena_begin_temp(model_arena);

    if (!process_ibl_textures(model_arena, model_arena, hdri_full_path, &world_pak->environment)) {
        log_error("Failed to process HDRI: %s", hdri_full_path);
        return false;
    }

    stream_env_map(state, model_arena, &world_pak->environment.skybox_cubemap);
    stream_env_map(state, model_arena, &world_pak->environment.irradiance_map);
    stream_env_map(state, model_arena, &world_pak->environment.prefiltered_env_map);

    arena_end_temp(model_arena, ibl_mark);
    return true;
}

static void stream_layer_entities(layer_src_t* layer, pak_serializing_state_t* state) {
    platform_file_seek(state->out_file, state->entities_offset);

    for (entity_src_t* curr_ent = layer->entities; curr_ent != NULL; curr_ent = curr_ent->next) {
        pak_entity_t pak_ent = {0};
        pak_ent.model_id     = curr_ent->model_id;

        float rx = DEG_TO_RAD(curr_ent->rotation.x);
        float ry = DEG_TO_RAD(curr_ent->rotation.y);
        float rz = DEG_TO_RAD(curr_ent->rotation.z);

        mat4_t t_mat = mat4_translate(curr_ent->position);
        mat4_t s_mat = mat4_scale(curr_ent->scale);
        mat4_t rot_x = mat4_rotate_x(rx);
        mat4_t rot_y = mat4_rotate_y(ry);
        mat4_t rot_z = mat4_rotate_z(rz);

        mat4_t r_mat      = mat4_mul(mat4_mul(rot_z, rot_x), rot_y);
        mat4_t sr_mat     = mat4_mul(r_mat, s_mat);
        pak_ent.transform = mat4_mul(t_mat, sr_mat);

        platform_file_write(state->out_file, &pak_ent, sizeof(pak_entity_t));
        state->entities_offset += sizeof(pak_entity_t);
    }
}

static bool stream_model(
    arena_t*                 model_arena,
    model_src_t*             curr_model,
    const char*              base_dir_cstr,
    pak_serializing_state_t* state
) {
    char model_path[1024];
    snprintf(
        model_path,
        sizeof(model_path),
        "%s/%.*s",
        base_dir_cstr,
        (int)curr_model->path.length,
        curr_model->path.data
    );

    arena_temp_t scratch_mark = arena_begin_temp(model_arena);
    scene_desc_t local_scene  = {0};

    if (!gltf_bake_model(
            model_arena,
            model_path,
            &local_scene,
            curr_model->id,
            curr_model->fast_textures,
            curr_model->z_up
        )) {
        log_error("Failed to bake model %s", model_path);
        return false;
    }

    if (local_scene.vertex_count > 0) {
        platform_file_seek(state->out_file, state->vertices_offset);
        uint64_t bytes = local_scene.vertex_count * sizeof(pak_vertex_t);
        platform_file_write(state->out_file, local_scene.vertices, bytes);
        state->vertices_offset += bytes;
    }

    if (local_scene.index_count > 0) {
        platform_file_seek(state->out_file, state->indices_offset);
        uint64_t bytes = local_scene.index_count * sizeof(uint32_t);
        platform_file_write(state->out_file, local_scene.indices, bytes);
        state->indices_offset += bytes;
    }

    if (local_scene.mesh_count > 0) {
        for (uint32_t m = 0; m < local_scene.mesh_count; m++) {
            local_scene.meshes[m].vertex_offset += state->total_vertices;
            local_scene.meshes[m].index_offset += state->total_indices;

            if (local_scene.meshes[m].base_color_texture_id >= 0) {
                local_scene.meshes[m].base_color_texture_id += state->total_textures;
            }
            if (local_scene.meshes[m].normal_texture_id >= 0) {
                local_scene.meshes[m].normal_texture_id += state->total_textures;
            }
            if (local_scene.meshes[m].ao_roughness_metallic_texture_id >= 0) {
                local_scene.meshes[m].ao_roughness_metallic_texture_id += state->total_textures;
            }
        }

        platform_file_seek(state->out_file, state->meshes_offset);
        uint64_t bytes = local_scene.mesh_count * sizeof(pak_mesh_t);
        platform_file_write(state->out_file, local_scene.meshes, bytes);
        state->meshes_offset += bytes;
    }

    if (local_scene.texture_count > 0) {
        for (uint32_t t = 0; t < local_scene.texture_count; t++) {
            if (local_scene.raw_texture_bytes[t] != NULL) {
                local_scene.textures[t].byte_offset = state->payloads_offset;
                platform_file_seek(state->out_file, state->payloads_offset);
                platform_file_write(
                    state->out_file,
                    local_scene.raw_texture_bytes[t],
                    local_scene.textures[t].byte_size
                );
                state->payloads_offset += local_scene.textures[t].byte_size;
            } else {
                local_scene.textures[t].byte_offset = 0;
            }
        }
        platform_file_seek(state->out_file, state->textures_offset);
        uint64_t bytes = local_scene.texture_count * sizeof(pak_texture_t);
        platform_file_write(state->out_file, local_scene.textures, bytes);
        state->textures_offset += bytes;
    }

    state->total_vertices += local_scene.vertex_count;
    state->total_indices += local_scene.index_count;
    state->total_textures += local_scene.texture_count;

    arena_end_temp(model_arena, scratch_mark);
    return true;
}

static void log_final_toc(
    world_pak_t*             world_pak,
    scene_manifest_t*        manifest,
    pak_serializing_state_t* state
) {
    uint64_t sz_verts    = world_pak->vertex_count * sizeof(pak_vertex_t);
    uint64_t sz_idxs     = world_pak->index_count * sizeof(uint32_t);
    uint64_t sz_meshes   = world_pak->mesh_count * sizeof(pak_mesh_t);
    uint64_t sz_tex_hdr  = world_pak->texture_count * sizeof(pak_texture_t);
    uint64_t sz_payloads = state->payloads_offset - manifest->offset_texture_payloads;
    uint64_t sz_total    = state->payloads_offset;

    log_info("Pak file:");
    log_info(
        "Magic: 0x%X, Version: %u, Type: %u",
        world_pak->magic,
        world_pak->version,
        world_pak->scene_type
    );
    log_info(
        "Counts: Vertices: %u, Indices: %u, Meshes: %u, Textures: %u, Entities: %u",
        world_pak->vertex_count,
        world_pak->index_count,
        world_pak->mesh_count,
        world_pak->texture_count,
        world_pak->entity_count
    );
    log_info(
        "Offsets: Vertices: %llu, Indices: %llu, Meshes: %llu, Textures: %llu, Entites: %llu",
        (unsigned long long)world_pak->vertex_offset,
        (unsigned long long)world_pak->index_offset,
        (unsigned long long)world_pak->mesh_offset,
        (unsigned long long)world_pak->texture_offset,
        (unsigned long long)world_pak->entity_offset
    );
    log_info(
        "Sizes: Vertices: %.1f MB, Indices: %.1f MB, Meshes: %.1f KB, Textures: %.1f KB",
        (double)sz_verts / (1024.0 * 1024.0),
        (double)sz_idxs / (1024.0 * 1024.0),
        (double)sz_meshes / 1024.0,
        (double)sz_tex_hdr / 1024.0
    );
    log_info(
        "Image Data: %.2f GB (%.1f MB)",
        (double)sz_payloads / (1024.0 * 1024.0 * 1024.0),
        (double)sz_payloads / (1024.0 * 1024.0)
    );
    log_info("Total: File Size: %.2f GB", (double)sz_total / (1024.0 * 1024.0 * 1024.0));
}

bool pass_2_stream_scene(
    arena_t*          model_arena,
    world_src_t*      world,
    string_span_t     base_dir,
    scene_manifest_t* manifest,
    const char*       output_file
) {
    char base_dir_cstr[512];
    snprintf(base_dir_cstr, sizeof(base_dir_cstr), "%.*s", (int)base_dir.length, base_dir.data);

    pak_serializing_state_t state = {0};
    state.out_file                = platform_file_open_write(output_file);
    if (!state.out_file) {
        log_error("Failed to open output file: %s", output_file);
        return false;
    }

    world_pak_t world_pak    = {0};
    world_pak.magic          = PAK_MAGIC;
    world_pak.version        = 1;
    world_pak.scene_type     = 1;
    world_pak.vertex_count   = manifest->total_vertices;
    world_pak.index_count    = manifest->total_indices;
    world_pak.mesh_count     = manifest->total_meshes;
    world_pak.texture_count  = manifest->total_textures;
    world_pak.entity_count   = manifest->total_entities;
    world_pak.vertex_offset  = manifest->offset_vertices;
    world_pak.index_offset   = manifest->offset_indices;
    world_pak.mesh_offset    = manifest->offset_meshes;
    world_pak.texture_offset = manifest->offset_textures;
    world_pak.entity_offset  = manifest->offset_entities;

    world_pak.environment.ambient_tint  = world->environment.ambient_tint;
    world_pak.environment.fog_density   = world->environment.fog_density;
    world_pak.environment.sun_direction = world->environment.sun_direction;
    world_pak.environment.sun_color     = world->environment.sun_color;
    world_pak.environment.sun_intensity = world->environment.sun_intensity;

    platform_file_write(state.out_file, &world_pak, sizeof(world_pak_t));

    pak_section_header_t vertices_chunk_header = {0};
    snprintf(vertices_chunk_header.magic, sizeof(vertices_chunk_header.magic), "VERTICES");
    platform_file_seek(state.out_file, manifest->offset_vertices);
    platform_file_write(state.out_file, &vertices_chunk_header, sizeof(pak_section_header_t));

    pak_section_header_t indices_chunk_header = {0};
    snprintf(indices_chunk_header.magic, sizeof(indices_chunk_header.magic), "INDICES");
    platform_file_seek(state.out_file, manifest->offset_indices);
    platform_file_write(state.out_file, &indices_chunk_header, sizeof(pak_section_header_t));

    pak_section_header_t meshes_chunk_header = {0};
    snprintf(meshes_chunk_header.magic, sizeof(meshes_chunk_header.magic), "MESHES");
    platform_file_seek(state.out_file, manifest->offset_meshes);
    platform_file_write(state.out_file, &meshes_chunk_header, sizeof(pak_section_header_t));

    pak_section_header_t textures_chunk_header = {0};
    snprintf(textures_chunk_header.magic, sizeof(textures_chunk_header.magic), "TEXTURES");
    platform_file_seek(state.out_file, manifest->offset_textures);
    platform_file_write(state.out_file, &textures_chunk_header, sizeof(pak_section_header_t));

    pak_section_header_t entites_chunk_header = {0};
    snprintf(entites_chunk_header.magic, sizeof(entites_chunk_header.magic), "ENTITIES");
    platform_file_seek(state.out_file, manifest->offset_entities);
    platform_file_write(state.out_file, &entites_chunk_header, sizeof(pak_section_header_t));

    state.vertices_offset = sizeof(pak_section_header_t) + manifest->offset_vertices;
    state.indices_offset  = sizeof(pak_section_header_t) + manifest->offset_indices;
    state.meshes_offset   = sizeof(pak_section_header_t) + manifest->offset_meshes;
    state.textures_offset = sizeof(pak_section_header_t) + manifest->offset_textures;
    state.payloads_offset = sizeof(pak_section_header_t) + manifest->offset_texture_payloads;
    state.entities_offset = sizeof(pak_section_header_t) + manifest->offset_entities;

    if (!process_and_stream_environment(model_arena, world, &world_pak, base_dir_cstr, &state)) {
        platform_file_close(state.out_file);
        return false;
    }

    for (uint32_t i = 0; i < world->layer_count; i++) {
        layer_src_t* layer = &world->layers[i];

        stream_layer_entities(layer, &state);

        for (model_src_t* curr_model = layer->models; curr_model != NULL;
             curr_model              = curr_model->next) {
            if (!stream_model(model_arena, curr_model, base_dir_cstr, &state)) {
                platform_file_close(state.out_file);
                return false;
            }
        }
    }

    log_final_toc(&world_pak, manifest, &state);

    platform_file_seek(state.out_file, 0);
    platform_file_write(state.out_file, &world_pak, sizeof(world_pak_t));

    platform_file_close(state.out_file);
    return true;
}

bool cook_world(
    arena_t*    scratch_arena,
    const char* world_input_file,
    const char* world_output_file
) {
    log_info("cook_world: input: '%s', output: '%s'", world_input_file, world_output_file);

    char base_dir[512];
    path_extract_base_dir(world_input_file, base_dir, sizeof(base_dir));
    string_span_t base_dir_span = span_init(base_dir, strnlen(base_dir, sizeof(base_dir)));

    string_span_t world_span;
    if (!arena_read_file_to_span(scratch_arena, world_input_file, &world_span)) {
        log_error("Failed to load file into arena: %s", world_input_file);
        return false;
    }

    world_src_t* world_source = (world_src_t*)arena_push_zero(scratch_arena, sizeof(world_src_t));
    if (!world_source) {
        log_error("Arena out of memory allocating world_source");
        return false;
    }

    if (!parse_world_source(scratch_arena, world_span, base_dir_span, world_source)) {
        log_error("Failed to parse world source");
        return false;
    }

    print_world_source(world_source);

    scene_manifest_t manifest = {};
    if (!pass_1_measure_scene(scratch_arena, world_source, base_dir_span, &manifest)) {
        log_error("Failed to measure scene");
        return false;
    }

    size_t model_capacity = 8ULL * 1024 * 1024 * 1024;
    void*  model_memory   = malloc(model_capacity);
    if (!model_memory) {
        log_error("CRITICAL: Failed to allocate %llu bytes for model arena", model_capacity);
        return false;
    }

    arena_t model_arena;
    arena_init(&model_arena, model_memory, model_capacity);

    if (!pass_2_stream_scene(
            &model_arena, world_source, base_dir_span, &manifest, world_output_file
        )) {
        log_error("Pass 2 Failed.");
        free(model_memory);
        return false;
    }

    free(model_memory);
    log_info("World processing complete!");
    return true;
}
