#pragma once

#include <stdint.h>

#include "libs/core/string_span.h"
#include "libs/tools/cooker/parsers/layer_source_parser.h"
#include "shared/scene_types.h"

typedef struct {
    string_span_t name;
    bool          is_active;
    string_span_t skybox_path;
    vec3_t        ambient_tint;
    float         fog_density;
} environment_src_t;

typedef struct {
    string_span_t name;
    char          version[256];
    char          source_file[512];

    environment_src_t environment;

    layer_src_t* layers;
    uint32_t     layer_count;

    pak_entity_t entities[PAK_MAX_ENTITIES];
    uint32_t     entity_count;
} world_src_t;

bool parse_world_source(
    arena_t*      arena,
    string_span_t text,
    string_span_t base_dir,
    world_src_t*  out_world_src
);

void print_world_source(const world_src_t* world_src);

scene_desc_t* process_world_source(
    arena_t*      pak_arena,
    arena_t*      scratch_arena,
    world_src_t*  world_source,
    string_span_t base_dir_span,
    world_pak_t*  out_world_pak
);

bool write_world_pak(arena_t* pak_arena, scene_desc_t* staged_scene, const char* output_file);
