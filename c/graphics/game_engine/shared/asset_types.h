#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "shared/math_types.h"
#include "shared/scene_types.h"

#define ASSET_MAX_PATH 256
#define ASSET_MAX_NAME 64
#define ASSET_MAX_MATERIALS 8
#define MAX_LEVEL_ENTITIES 1024
#define MAX_WORLD_CHUNKS 1024
#define MAX_BUNDLE_ITEMS 256

typedef struct {
    char albedo_map[ASSET_MAX_PATH];
    char normal_map[ASSET_MAX_PATH];
    char rma_map[ASSET_MAX_PATH]; // Roughness/Metallic/AO

    vec4_t base_color;
    float  roughness_factor;
    float  metallic_factor;
} material_desc_t;

typedef struct {
    char source_file[ASSET_MAX_PATH];
    char mesh_name[ASSET_MAX_NAME]; // Targets the specific mesh in the glTF

    uint32_t material_count;
    char     materials[ASSET_MAX_MATERIALS][ASSET_MAX_PATH];
} model_desc_t;

typedef struct {
    char name[ASSET_MAX_NAME];
    char template_path[ASSET_MAX_PATH]; // Points to .model or .tpl

    vec3_t position;
    vec3_t rotation;
    vec3_t scale;
} entity_desc_t;

typedef struct {
    vec3_t sun_direction;
    vec3_t sun_color;
    char   skybox_path[ASSET_MAX_PATH];

    uint32_t      entity_count;
    entity_desc_t entities[MAX_LEVEL_ENTITIES];
} level_desc_t;

typedef struct {
    float chunk_size;
    float load_radius;
    float unload_radius;

    uint32_t chunk_count;
    char     chunk_paths[MAX_WORLD_CHUNKS][ASSET_MAX_PATH];
} world_desc_t;

typedef struct {
    char bundle_name[ASSET_MAX_NAME];

    uint32_t item_count;
    char     items[MAX_BUNDLE_ITEMS][ASSET_MAX_PATH];
} asset_bundle_desc_t;
