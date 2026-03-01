#include <SDL3/SDL.h>
#include "logger.h"
#include "renderer.h"
#include "sdl_backend.h"


int main(int argc, char* argv[]) {
    log_init();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return 1;

    platform_t* platform = platform_create("Vulkan Triangle", 800, 600);
    int w, h;
    platform_get_window_size(platform, &w, &h);

    renderer_t* renderer = renderer_create(platform_get_window(platform), w, h);
    if (renderer == nullptr) {
        log_error("Main: Renderer creation failed, exiting...");
        platform_destroy(platform);
        return 1;
    }

    bool running = true;
    while (running) {
        running = platform_process_events(platform);
        renderer_draw(renderer);
    }

    log_info("Main: Closing down...");
    renderer_destroy(renderer);
    platform_destroy(platform);
    SDL_Quit();
    return 0;
}
