#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include "core/math/math_types.h"

typedef struct {
    float  pos[3];
    vec4_t color;
    float  uv[2];
    // float normal[3];
} vertex_t;

typedef struct {
    vertex_t* vertices;
    uint32_t  vertex_count;
} mesh_data_t;

#endif
