#ifndef RENDERER_H
#define RENDERER_H

struct SDL_Window;
typedef struct SDL_Window SDL_Window;

typedef struct renderer_t renderer_t;

renderer_t* renderer_create(SDL_Window* window, int width, int height);
void renderer_destroy(renderer_t* r);
void renderer_draw(renderer_t* r);

#endif
