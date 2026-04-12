
#ifndef PAK_FORMAT_H
#define PAK_FORMAT_H

#include <stdint.h>

// #include "core/math/math_types.h"
#include "math/math_types.h"

#define PAK_MAGIC 0x4B415057 // "WPAK"

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t chunk_count;
} pak_header_t;

// TOOD: fix this up, replace all pak_vertex_t references with vertex_t
typedef vertex_t pak_vertex_t;

typedef struct {
    uint32_t model_id;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t index_offset;
    uint32_t index_count;
    int32_t  texture_id;
} pak_mesh_t;

typedef struct {
    uint32_t chunk_id;
    uint32_t entity_count;
    uint32_t mesh_count;
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t texture_count;
} pak_chunk_header_t;

#define PAK_TEX_FORMAT_RGBA8 0
#define PAK_TEX_FORMAT_PNG 1

typedef struct {
    uint32_t format;
    uint64_t byte_offset;
    uint32_t byte_size;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
} pak_texture_t;

typedef struct {
    uint32_t model_id;
    mat4_t   transform;
} pak_entity_t;

#endif
