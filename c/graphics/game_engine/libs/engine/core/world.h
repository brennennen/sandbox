#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>

#include "engine/modules/assets/image.h"
#include "shared/pak_format.h"


#define MAX_LOADED_ENTITIES 1024
#define MAX_LOADED_MESHES 1024
#define MAX_LOADED_VERTICES 5000000
#define MAX_LOADED_INDICES 15000000

#define MAX_LOADED_TEXTURES 256

extern pak_entity_t loaded_entities[MAX_LOADED_ENTITIES];
extern uint32_t     loaded_entity_count;

extern pak_mesh_t loaded_meshes[MAX_LOADED_MESHES];
extern uint32_t   loaded_mesh_count;

extern pak_vertex_t loaded_vertices[MAX_LOADED_VERTICES];
extern uint32_t     loaded_vertices_count;

extern uint32_t loaded_indices[MAX_LOADED_INDICES];
extern uint32_t loaded_indices_count;

extern pak_texture_t loaded_textures[MAX_LOADED_TEXTURES];
extern uint32_t      loaded_texture_count;

bool world_load_chunk(uint32_t chunk_id);
bool world_load_texture_image(uint32_t tex_idx, image_t* out_img);

#endif
