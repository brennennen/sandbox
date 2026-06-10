#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine/core/logger.h"
#include "shared/scene_types.h"
#include "tools/cooker/cooker.h"
#include "tools/core/tools_core.h"
#include "tools/parsers/scene_parser.h"

// todo: figure out how to handle platform stuff the logger is dependent on.
// void* platform_mutex_create(void) { return (void*)1; }
// void  platform_mutex_destroy(void* mutex) { (void)mutex; }
// void  platform_mutex_lock(void* mutex) { (void)mutex; }
// void  platform_mutex_unlock(void* mutex) { (void)mutex; }

static void extract_base_dir(const char* input_file, char* base_dir, size_t max_len) {
    strncpy(base_dir, ".", max_len);
    const char* last_slash     = strrchr(input_file, '/');
    const char* last_backslash = strrchr(input_file, '\\');
    const char* slash          = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (slash) {
        size_t dir_len = slash - input_file;
        if (dir_len < max_len) {
            strncpy(base_dir, input_file, dir_len);
            base_dir[dir_len] = '\0';
        }
    }
}

static char* read_file_to_string(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

static int64_t write_pak_file(const char* output_file, scene_desc_t* flattened_scene) {
    FILE* pak = fopen(output_file, "wb");
    if (!pak) {
        log_error("Failed to open output file for writing.");
        return -1;
    }

    pak_header_t header = {.magic = PAK_MAGIC, .version = 1, .chunk_count = 1};
    fwrite(&header, sizeof(pak_header_t), 1, pak);

    pak_chunk_header_t chunk = {
        .chunk_id      = 0,
        .entity_count  = flattened_scene->entity_count,
        .mesh_count    = flattened_scene->mesh_count,
        .vertex_count  = flattened_scene->vertex_count,
        .index_count   = flattened_scene->index_count,
        .texture_count = flattened_scene->texture_count
    };
    fwrite(&chunk, sizeof(pak_chunk_header_t), 1, pak);
    fwrite(&flattened_scene->environment, sizeof(environment_desc_t), 1, pak);
    fwrite(flattened_scene->entities, sizeof(pak_entity_t), chunk.entity_count, pak);
    fwrite(flattened_scene->meshes, sizeof(pak_mesh_t), chunk.mesh_count, pak);
    fwrite(flattened_scene->vertices, sizeof(pak_vertex_t), chunk.vertex_count, pak);
    fwrite(flattened_scene->indices, sizeof(uint32_t), chunk.index_count, pak);

    int64_t texture_header_start = _ftelli64(pak);
    fwrite(flattened_scene->textures, sizeof(pak_texture_t), chunk.texture_count, pak);

    for (uint32_t i = 0; i < chunk.texture_count; i++) {
        flattened_scene->textures[i].byte_offset = (uint64_t)_ftelli64(pak);
        fwrite(
            flattened_scene->raw_texture_bytes[i], 1, flattened_scene->textures[i].byte_size, pak
        );
        free(flattened_scene->raw_texture_bytes[i]);
    }

    _fseeki64(pak, texture_header_start, SEEK_SET);
    fwrite(flattened_scene->textures, sizeof(pak_texture_t), chunk.texture_count, pak);

    _fseeki64(pak, 0, SEEK_END);
    int64_t pak_size_bytes = _ftelli64(pak);
    fclose(pak);

    return pak_size_bytes;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        log_error("Usage: cooker <input.world> <output.pak>");
        return 1;
    }

    clock_t start_time = clock();
    tools_init();
    log_info("Initializing Level Cooker...");

    const char* input_file  = argv[1];
    const char* output_file = argv[2];

    // new:
    void*   build_memory = malloc(1024 * 1024 * 1024);
    arena_t build_arena;
    arena_init(&build_arena, build_memory, 1024 * 1024 * 1024);
    cook_world(&build_arena, input_file, output_file);
    return 0;

}
