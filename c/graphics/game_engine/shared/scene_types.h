#ifndef SCENE_TYPES_H
#define SCENE_TYPES_H

#include <stdint.h>

#include "pak_format.h"

#define PAK_MAX_ENTITIES 1024
#define PAK_MAX_MESHES 1024
#define PAK_MAX_VERTICES 5000000
#define PAK_MAX_INDICES 15000000
#define PAK_MAX_TEXTURES 256

#define GRAPHICS_INVALID_HANDLE UINT32_MAX

typedef enum {
    PAK_TEX_FORMAT_UNKNOWN = 0,
    PAK_TEX_FORMAT_RGBA8_UNORM, // Linear math data (Normal Maps, Roughness, Metallic)
    PAK_TEX_FORMAT_RGBA8_SRGB,  // Gamma-corrected color data (Albedo/Base Color)
    PAK_TEX_FORMAT_R8_UNORM,    // Single-channel data (Grayscale masks, heightmaps)
    PAK_TEX_FORMAT_PNG_UNORM,
    PAK_TEX_FORMAT_PNG_SRGB,
    // PAK_TEX_FORMAT_BC7_SRGB, // block compression
    PAK_TEX_FORMAT_COUNT
} pak_texture_format_t;

typedef struct {
    pak_entity_t entities[PAK_MAX_ENTITIES];
    uint32_t entity_count;

    pak_mesh_t meshes[PAK_MAX_MESHES];
    uint32_t mesh_count;

    pak_vertex_t vertices[PAK_MAX_VERTICES];
    uint32_t vertex_count;

    uint32_t indices[PAK_MAX_INDICES];
    uint32_t index_count;

    pak_texture_t textures[PAK_MAX_TEXTURES];
    uint8_t* raw_texture_bytes[PAK_MAX_TEXTURES];
    uint32_t texture_count;
} scene_desc_t;

#endif // SCENE_TYPES_H
