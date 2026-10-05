#pragma once

#include <stdalign.h>
#include <stdint.h>

#include "pak_format.h"

#define PAK_MAX_ENTITIES 1024
#define PAK_MAX_MESHES 1024
#define PAK_MAX_VERTICES 10000000
#define PAK_MAX_INDICES 15000000
#define PAK_MAX_TEXTURES 512

#define GRAPHICS_INVALID_HANDLE UINT32_MAX

typedef struct {
    bool  is_active;
    char  skybox_path[256];
    float exposure;
    alignas(16) vec3_t ambient_tint;
    alignas(16) vec3_t sun_direction;
    alignas(16) vec3_t sun_colo;
    float sun_intensity;
    float fog_density;
    alignas(16) vec3_t fog_color;
    alignas(16) vec3_t gravity;
} environment_desc_t;

// typedef struct {
//     bool   is_active;
//     char   skybox_path[256];
//     float  exposure;
//     vec3_t ambient_tint;
//     vec3_t sun_direction;
//     vec3_t sun_colo;
//     float  sun_intensity;
//     float  fog_density;
//     vec3_t fog_color;
//     vec3_t gravity;
// } environment_t;

typedef struct {
    environment_desc_t environment;

    pak_entity_t* entities;
    uint32_t      entity_count;

    pak_mesh_t* meshes;
    uint32_t    mesh_count;

    pak_vertex_t* vertices;
    uint32_t      vertex_count;

    uint32_t* indices;
    uint32_t  index_count;

    pak_texture_t* textures;
    uint8_t**      raw_texture_bytes;
    uint32_t       texture_count;

    char (*texture_cache_paths)[256];

} scene_desc_t;

typedef struct {
    uint32_t total_vertices;
    uint32_t total_indices;
    uint32_t total_meshes;
    uint32_t total_entities;
    uint32_t total_textures;

    uint64_t offset_vertices;
    uint64_t offset_indices;
    uint64_t offset_meshes;
    uint64_t offset_entities;
    uint64_t offset_textures;
    uint64_t offset_texture_payloads;
} scene_manifest_t;
