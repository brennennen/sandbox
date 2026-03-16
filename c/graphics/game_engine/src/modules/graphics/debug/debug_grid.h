#ifndef DEBUG_GRID_H
#define DEBUG_GRID_H

#include "core/math/math_types.h"

typedef struct {
    vec3_t pos;
    vec3_t color;
} grid_vertex_t;

void generate_grid(grid_vertex_t* vertices, int size, float step);

#endif // DEBUG_GRID_H
