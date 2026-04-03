#include <stdint.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "core/logger.h"

#include "platform/platform.h"

#define MAX_KEYS 512 // SDL_NUM_SCANCODES is 512

struct platform_t {
    SDL_Window* window;
    float       mouse_dx;
    float       mouse_dy;

    const bool* current_keys;
    bool        previous_keys[MAX_KEYS];
};

static SDL_Scancode key_map[KEY_MAX] = {
    [KEY_W]      = SDL_SCANCODE_W,
    [KEY_A]      = SDL_SCANCODE_A,
    [KEY_S]      = SDL_SCANCODE_S,
    [KEY_D]      = SDL_SCANCODE_D,
    [KEY_SPACE]  = SDL_SCANCODE_SPACE,
    [KEY_LSHIFT] = SDL_SCANCODE_LSHIFT,
    [KEY_P]      = SDL_SCANCODE_P,
    [KEY_ESCAPE] = SDL_SCANCODE_ESCAPE,
    [KEY_F1]     = SDL_SCANCODE_F1,
    [KEY_F2]     = SDL_SCANCODE_F2,
    [KEY_F3]     = SDL_SCANCODE_F3,
    [KEY_F4]     = SDL_SCANCODE_F4,
    [KEY_F5]     = SDL_SCANCODE_F5,
    [KEY_F12]    = SDL_SCANCODE_F12
};

platform_t* platform_create(const char* title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        log_error("SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }

    platform_t* platform     = calloc(1, sizeof(struct platform_t));
    uint32_t    window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    platform->window = SDL_CreateWindow(title, width, height, window_flags);
    if (!platform->window) {
        log_error("SDL_CreateWindow failed");
        free(platform);
        SDL_Quit();
        return NULL;
    }

    platform->current_keys = SDL_GetKeyboardState(NULL);
    memset(platform->previous_keys, 0, sizeof(platform->previous_keys));

    log_info("platform: window created successfully");
    return platform;
}

void platform_destroy(platform_t* p) {
    if (p) {
        SDL_DestroyWindow(p->window);
        free(p);
    }
    SDL_Quit();
    log_info("platform: cleaned up");
}

bool platform_update(platform_t* platform) {
    memcpy(platform->previous_keys, platform->current_keys, sizeof(platform->previous_keys));
    platform->mouse_dx = 0.0f;
    platform->mouse_dy = 0.0f;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            platform->mouse_dx += event.motion.xrel;
            platform->mouse_dy += event.motion.yrel;
        }
    }
    return true;
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

void platform_get_window_size(platform_t* p, int* w, int* h) { SDL_GetWindowSize(p->window, w, h); }

void platform_set_title(platform_t* p, const char* title) { SDL_SetWindowTitle(p->window, title); }

void platform_set_relative_mouse(platform_t* p, bool enable) {
    SDL_SetWindowRelativeMouseMode(p->window, enable);
}

void* platform_get_native_window(platform_t* p) { return p->window; }

void platform_get_mouse_delta(platform_t* p, float* dx, float* dy) {
    *dx = p->mouse_dx;
    *dy = p->mouse_dy;
}

uint64_t platform_get_ticks(platform_t* p) { return SDL_GetTicks(); }

bool platform_get_key(platform_t* platform, platform_key_t key) {
    if (key >= KEY_MAX)
        return false;
    return platform->current_keys[key_map[key]];
}

bool platform_get_key_pressed(platform_t* platform, platform_key_t key) {
    if (key >= KEY_MAX)
        return false;
    SDL_Scancode sdl_key = key_map[key];
    return platform->current_keys[sdl_key] && !platform->previous_keys[sdl_key];
}

bool platform_get_key_released(platform_t* platform, platform_key_t key) {
    if (key >= KEY_MAX)
        return false;
    SDL_Scancode sdl_key = key_map[key];
    return !platform->current_keys[sdl_key] && platform->previous_keys[sdl_key];
}

// #ifdef RENDERER_VULKAN
bool platform_create_vulkan_surface(platform_t* p, VkInstance instance, VkSurfaceKHR* surface) {
    return SDL_Vulkan_CreateSurface(p->window, instance, NULL, surface);
}

const char* const* platform_get_vulkan_extensions(platform_t* p, uint32_t* count) {
    return SDL_Vulkan_GetInstanceExtensions(count);
}
// #endif
