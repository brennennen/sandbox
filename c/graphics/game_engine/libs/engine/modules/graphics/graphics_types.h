#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include <stdint.h>

#include "engine/core/math/math_types.h"

typedef struct {
    uint32_t id;
} material_handle_t;

typedef struct {
    vertex_t* vertices;
    uint32_t  vertex_count;

    uint32_t* indices;
    uint32_t  index_count;
} mesh_data_t;

typedef struct {
    uint32_t id;
} mesh_handle_t;

typedef struct {
    uint32_t id;
} texture_handle_t;

typedef struct {
    uint32_t          first_index;
    uint32_t          index_count;
    material_handle_t material;
} submesh_t;

typedef struct {
    mesh_handle_t    mesh;
    texture_handle_t texture;
    mat4_t           transform;
} render_object_t;

#endif
