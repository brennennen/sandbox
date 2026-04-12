#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"
#include "tools/core/scene_types.h"
#include "tools/parsers/scene_parser.h"

// todo: figure out how to handle platform stuff the logger is dependent on.
void* platform_mutex_create(void) { return (void*)1; }
void  platform_mutex_destroy(void* mutex) { (void)mutex; }
void  platform_mutex_lock(void* mutex) { (void)mutex; }
void  platform_mutex_unlock(void* mutex) { (void)mutex; }

int main(int argc, char** argv) {
    if (argc < 3) {
        log_error("Usage: cooker <input.world> <output.pak>");
        return 1;
    }

    log_info("Initializing Level Cooker...");

    const char* input_file  = argv[1];
    const char* output_file = argv[2];

    char* world_text = read_file_to_string(input_file);
    if (!world_text) {
        log_error("Failed to read input file.");
        return 1;
    }

    char        base_dir[512]  = ".";
    const char* last_slash     = strrchr(input_file, '/');
    const char* last_backslash = strrchr(input_file, '\\');
    const char* slash          = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (slash) {
        size_t dir_len = slash - input_file;
        if (dir_len < sizeof(base_dir)) {
            strncpy(base_dir, input_file, dir_len);
            base_dir[dir_len] = '\0';
        }
    }

    scene_desc_t* flattened_scene = calloc(1, sizeof(scene_desc_t));
    if (!flattened_scene) {
        log_error("Out of memory!");
        free(world_text);
        return 1;
    }

    parse_scene_file(world_text, base_dir, flattened_scene);
    free(world_text);

    log_info("Writing Bulk Data to %s...", output_file);
    FILE* pak = fopen(output_file, "wb");
    if (!pak) {
        log_error("Failed to open output file for writing.");
        return 1;
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
    fwrite(flattened_scene->entities, sizeof(pak_entity_t), chunk.entity_count, pak);
    fwrite(flattened_scene->meshes, sizeof(pak_mesh_t), chunk.mesh_count, pak);
    fwrite(flattened_scene->vertices, sizeof(pak_vertex_t), chunk.vertex_count, pak);
    fwrite(flattened_scene->indices, sizeof(uint32_t), chunk.index_count, pak);

    long texture_header_start = ftell(pak);
    fwrite(flattened_scene->textures, sizeof(pak_texture_t), chunk.texture_count, pak);

    for (uint32_t i = 0; i < chunk.texture_count; i++) {
        flattened_scene->textures[i].byte_offset = ftell(pak);
        fwrite(
            flattened_scene->raw_texture_bytes[i], 1, flattened_scene->textures[i].byte_size, pak
        );
        free(flattened_scene->raw_texture_bytes[i]);
    }

    fseek(pak, texture_header_start, SEEK_SET);
    fwrite(flattened_scene->textures, sizeof(pak_texture_t), chunk.texture_count, pak);

    fclose(pak);

    log_info("Success! Wrote Chunk 0 with %d verts.", chunk.vertex_count);

    free(flattened_scene);
    return 0;
}
