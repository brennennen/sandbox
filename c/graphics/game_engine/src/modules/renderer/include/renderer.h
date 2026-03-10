#ifndef RENDERER_H
#define RENDERER_H

#include "core/camera.h"
#include "core/math/math_types.h"

struct SDL_Window;
typedef struct SDL_Window SDL_Window;

/**
 *
 * Opaque pointer (PIMPL)
 */
typedef struct renderer_t renderer_t;

renderer_t* renderer_create(SDL_Window* window, int width, int height);

void renderer_destroy(renderer_t* r);

void renderer_draw(renderer_t* r, SDL_Window* window);

camera_t* renderer_get_camera(renderer_t* r);

#endif
