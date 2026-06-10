
#pragma once
#include <stdint.h>

// #include "core/math/math_types.h"
#include "math_types.h"

#define PAK_MAGIC 0x4B415057 // "WPAK"

typedef enum {
    PAK_TEX_FORMAT_UNKNOWN = 0,
    PAK_TEX_FORMAT_RGBA8_UNORM,
    PAK_TEX_FORMAT_RGBA8_SRGB,
    PAK_TEX_FORMAT_R8_UNORM,
    PAK_TEX_FORMAT_PNG_UNORM,
    PAK_TEX_FORMAT_PNG_SRGB,
    PAK_TEX_FORMAT_BC7_UNORM,
    PAK_TEX_FORMAT_BC7_SRGB,
    PAK_TEX_FORMAT_RGBA32F,
    PAK_TEX_FORMAT_COUNT
} pak_texture_format_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t format;
    uint64_t data_offset;
    uint64_t data_size;
} texture_pak_t;

typedef struct {
    vec3_t ambient_tint;
    float  fog_density;

    texture_pak_t skybox_cubemap;      // background
    texture_pak_t irradiance_map;      // diffuse ambient light
    texture_pak_t prefiltered_env_map; // specular reflections (mipmapped)

} environment_pak_t;

typedef struct {
    uint32_t          magic; // 'WPAK'
    uint32_t          version;
    environment_pak_t environment;

    uint64_t vertex_offset;
    uint32_t vertex_count;
    uint32_t _pad0;

    uint64_t index_offset;
    uint32_t index_count;
    uint32_t _pad1;

    uint64_t mesh_offset;
    uint32_t mesh_count;
    uint32_t _pad2;

    uint64_t entity_offset;
    uint32_t entity_count;
    uint32_t _pad3;

    uint64_t texture_offset;
    uint32_t texture_count;
    uint32_t _pad4;
} world_pak_t;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t chunk_count;
} pak_header_t;

typedef union {
    float data[2];
    struct {
        float u;
        float v;
    };
} pak_uv_t;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} pak_color_t;

typedef struct {
    vec3_t   pos;
    vec4_t   color;
    pak_uv_t uv;
    vec3_t   normal;
    vec4_t   tangent;
} pak_vertex_t;

// TOOD: fix this up, replace all pak_vertex_t references with vertex_t
// typedef vertex_t pak_vertex_t;

typedef struct {
    uint32_t model_id;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t index_offset;
    uint32_t index_count;

    // int32_t  texture_id;
    int32_t base_color_texture_id;
    int32_t normal_texture_id;
    int32_t ao_roughness_metallic_texture_id;
    bool    is_alpha_masked;

    vec3_t bounding_center;
    float  bounding_radius;

    float metallic_factor;
    float roughness_factor;

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
    uint32_t mip_levels;
} pak_texture_t;

typedef struct {
    uint32_t model_id;
    mat4_t   transform;
} pak_entity_t;
