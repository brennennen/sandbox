
#include "debug_grid.h"

void generate_grid(grid_vertex_t* vertices, int size, float step) {
    int   index     = 0;
    float half_size = (size * step) / 2.0f;

    for (int i = 0; i <= size; i++) {
        float pos = -half_size + (i * step);

        // Lines parallel to Z axis
        vertices[index++] = (grid_vertex_t){{pos, 0, -half_size}, {0.5f, 0.5f, 0.5f}};
        vertices[index++] = (grid_vertex_t){{pos, 0, half_size}, {0.5f, 0.5f, 0.5f}};

        // Lines parallel to X axis
        vertices[index++] = (grid_vertex_t){{-half_size, 0, pos}, {0.5f, 0.5f, 0.5f}};
        vertices[index++] = (grid_vertex_t){{half_size, 0, pos}, {0.5f, 0.5f, 0.5f}};
    }
}
