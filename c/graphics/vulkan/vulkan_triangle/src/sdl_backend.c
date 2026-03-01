#include "sdl_backend.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include "logger.h"

struct platform_t {
    SDL_Window* window;
};

platform_t* platform_create(const char* title, int width, int height) {
    platform_t* p = malloc(sizeof(struct platform_t));

    p->window = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN);
    if (!p->window) {
        log_error("SDL_CreateWindow failed");
        free(p);
        return NULL;
    }

    log_info("platform: window created successfully");
    return p;
}

bool platform_process_events(platform_t* p) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
    }
    return true;
}

struct SDL_Window* platform_get_window(platform_t* p) {
    return p->window;
}

void platform_get_window_size(platform_t* p, int* w, int* h) {
    SDL_GetWindowSize(p->window, w, h);
}

void platform_destroy(platform_t* p) {
    SDL_DestroyWindow(p->window);
    free(p);
    log_info("platform: cleaned up");
}
