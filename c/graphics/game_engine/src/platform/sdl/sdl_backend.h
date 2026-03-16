#ifndef SDL_BACKEND_H
#define SDL_BACKEND_H

#include <stdbool.h>

typedef struct platform_t platform_t;

platform_t* platform_create(const char* title, int width, int height);
void platform_destroy(platform_t* p);
struct SDL_Window* platform_get_window(platform_t* p);
void platform_get_window_size(platform_t* p, int* w, int* h);
bool platform_process_events(platform_t* p);

#endif
