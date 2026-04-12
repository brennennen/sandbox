#ifndef DEBUG_GRID_H
#define DEBUG_GRID_H

#include <stdint.h>

#include "engine/core/math/math_types.h"

typedef struct {
    vec3_t pos;
    vec3_t color;
} grid_vertex_t;

uint32_t debug_grid_vertex_count(int size);
void     generate_grid(vertex_t* vertices, int size, float step);

#endif // DEBUG_GRID_H
