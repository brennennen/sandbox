#ifndef VK_DEBUG_H
#define VK_DEBUG_H

#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"

void init_debug_grid(graphics_t* graphics);
void init_debug_frustum_buffer(graphics_t* graphics);

void init_debug_sun_line_buffer(graphics_t* graphics);
void graphics_update_debug_sun_line(graphics_t* graphics, vec3_t origin, vec3_t sun_direction);

#endif // VK_DEBUG_H
