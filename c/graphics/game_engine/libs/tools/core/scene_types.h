#ifndef SCENE_TYPES_H
#define SCENE_TYPES_H

#include <stdint.h>

#include "engine/core/pak_format.h"

#define MAX_ENTITIES 1024
#define MAX_MESHES 1024
#define MAX_VERTICES 5000000
#define MAX_INDICES 15000000
#define MAX_TEXTURES 256

typedef struct {
    pak_entity_t entities[MAX_ENTITIES];
    uint32_t     entity_count;

    pak_mesh_t meshes[MAX_MESHES];
    uint32_t   mesh_count;

    pak_vertex_t vertices[MAX_VERTICES];
    uint32_t     vertex_count;

    uint32_t indices[MAX_INDICES];
    uint32_t index_count;

    pak_texture_t textures[MAX_TEXTURES];
    uint8_t*      raw_texture_bytes[MAX_TEXTURES];
    uint32_t      texture_count;
} scene_desc_t;

#endif // SCENE_TYPES_H
