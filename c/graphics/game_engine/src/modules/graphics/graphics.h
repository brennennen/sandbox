#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "core/camera.h"
#include "core/math/math_types.h"

#include "platform/platform.h"

/**
 *
 * Opaque pointer (PIMPL)
 */
typedef struct renderer_t renderer_t;

renderer_t* renderer_create(platform_t* platform, int width, int height);

void renderer_destroy(renderer_t* r);

void renderer_draw(renderer_t* r, platform_t* platform, mat4_t view);

#endif
