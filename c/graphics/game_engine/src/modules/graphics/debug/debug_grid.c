
#include "modules/graphics/graphics_types.h"

#include "core/math/math_types.h"

#include "debug_grid.h"

uint32_t debug_grid_vertex_count(int size) {
    return 4 * (size + 1); // (size + 1) lines per axis * 2 axes * 2 points per line
}

void generate_grid(vertex_t* vertices, int size, float step) {
    int   index     = 0;
    float half_size = (size * step) / 2.0f;

    for (int i = 0; i <= size; i++) {
        float pos = -half_size + (i * step);

        vec4_t color_x = {0.3f, 0.3f, 0.3f, 1.0f};
        if (pos == 0.0f) {
            color_x = (vec4_t){1.0f, 0.0f, 0.0f, 1.0f}; // X-axis
        }
        vertices[index++] = (vertex_t){{-half_size, pos, 0.0f}, color_x, {0.0f, 0.0f}};
        vertices[index++] = (vertex_t){{half_size, pos, 0.0f}, color_x, {0.0f, 0.0f}};

        vec4_t color_y = {0.3f, 0.3f, 0.3f, 1.0f};
        if (pos == 0.0f) {
            color_y = (vec4_t){0.0f, 1.0f, 0.0f, 1.0f}; // Y-axis
        }
        vertices[index++] = (vertex_t){{pos, -half_size, 0.0f}, color_y, {0.0f, 0.0f}};
        vertices[index++] = (vertex_t){{pos, half_size, 0.0f}, color_y, {0.0f, 0.0f}};
    }
}
