#ifndef OBJ_H
#define OBJ_H

#include "modules/graphics/vulkan/vk_types.h" // For vertex_t
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    vertex_t* vertices;
    uint32_t  vertex_count;
} mesh_data_t;

// Loads a triangulated OBJ file.
// Requires the OBJ to be exported with UVs and Normals (v/vt/vn format).
bool load_obj(const char* filepath, mesh_data_t* out_mesh);
void free_mesh(mesh_data_t* mesh);

#endif
