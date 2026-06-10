#pragma once

#include <stdint.h>

#include "libs/core/string_span.h"
#include "shared/scene_types.h"

typedef struct entity_src_t entity_src_t;
struct entity_src_t {
    string_span_t name;
    uint32_t      model_id;
    vec3_t        position;
    vec3_t        rotation;
    vec3_t        scale;
    entity_src_t* next;
};

typedef struct model_src_t model_src_t;
struct model_src_t {
    string_span_t path;
    uint32_t      id;
    bool          fast_textures;
    bool          z_up;
    model_src_t*  next;
};

typedef struct {
    string_span_t name;
    string_span_t version;
    string_span_t path;

    model_src_t* models;
    uint32_t     model_count;

    entity_src_t* entities;
    uint32_t      entity_count;
} layer_src_t;

bool parse_layer_source(
    arena_t*      arena,
    string_span_t text,
    string_span_t base_dir,
    layer_src_t*  out_layer_src
);
