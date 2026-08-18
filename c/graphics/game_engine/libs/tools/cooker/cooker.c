#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cooker.h"
#include "engine/platform/platform.h"
#include "libs/engine/core/logger.h"
#include "libs/tools/cooker/world_src.h"
#include "libs/tools/parsers/parser_source_types.h"


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

static void read_file_to_span(const char* filepath, string_span_t* out_span) {
    platform_file_t f = platform_file_open_read(filepath);
    if (!f)
        return;

    uint64_t length = platform_file_size(f);
    char*    buffer = malloc(length + 1);

    if (buffer) {
        platform_file_read(f, buffer, length);
        buffer[length]   = '\0';
        out_span->data   = buffer;
        out_span->length = length;
    }

    platform_file_close(f);
}

static void read_file_to_span2(const char* filepath, string_span_t* out_span) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        return;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length]   = '\0';
    out_span->data   = buffer;
    out_span->length = length;
    fclose(f);
    return;
}

bool cook_world(
    arena_t*    scratch_arena,
    const char* world_input_file,
    const char* world_output_file
) {
    log_info("cook_world: input: '%s', output: '%s'", world_input_file, world_output_file);
    char base_dir[512];
    extract_base_dir(world_input_file, base_dir, sizeof(base_dir));
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
    size_t pak_capacity = 4ULL * 1024 * 1024 * 1024;
    void*  pak_memory   = malloc(pak_capacity);
    if (!pak_memory) {
        log_error("CRITICAL: OS refused to allocate %llu bytes for scratch arena!", pak_capacity);
        return false;
    }
    arena_t pak_arena;
    arena_init(&pak_arena, pak_memory, pak_capacity);

    world_pak_t world_pak;

    scene_desc_t* staged_scene = process_world_source(
        &pak_arena, scratch_arena, world_source, base_dir_span, &world_pak
    );
    if (!staged_scene) {
        log_error("failed: process_world_source");
        return false;
    }

    if (!write_world_pak(&pak_arena, staged_scene, world_output_file)) {
        return false;
    }

    return true;
}
