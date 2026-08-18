#pragma once

#include <stdalign.h>
#include <stdint.h>

#include "pak_format.h"

#define PAK_MAX_ENTITIES 1024
#define PAK_MAX_MESHES 1024
#define PAK_MAX_VERTICES 5000000
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

typedef struct {
    bool   is_active;
    char   skybox_path[256];
    float  exposure;
    vec3_t ambient_tint;
    vec3_t sun_direction;
    vec3_t sun_colo;
    float  sun_intensity;
    float  fog_density;
    vec3_t fog_color;
    vec3_t gravity;
} environment_t;

typedef struct {
    environment_desc_t environment;

    pak_entity_t entities[PAK_MAX_ENTITIES];
    uint32_t     entity_count;

    pak_mesh_t meshes[PAK_MAX_MESHES];
    uint32_t   mesh_count;

    pak_vertex_t vertices[PAK_MAX_VERTICES];
    uint32_t     vertex_count;

    uint32_t indices[PAK_MAX_INDICES];
    uint32_t index_count;

    pak_texture_t textures[PAK_MAX_TEXTURES];
    uint8_t*      raw_texture_bytes[PAK_MAX_TEXTURES];
    uint32_t      texture_count;

    char texture_cache_paths[PAK_MAX_TEXTURES][256];

} scene_desc_t;
