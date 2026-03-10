
#include <math.h>

#include "core/logger.h"
#include "platform/sdl_backend.h"
#include "renderer.h"
#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
    log_init();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return 1;
    }

    platform_t* platform = platform_create("Game Engine", 800, 600);

    int w;
    int h;

    platform_get_window_size(platform, &w, &h);

    renderer_t* renderer = renderer_create(platform_get_window(platform), w, h);
    if (renderer == nullptr) {
        log_error("Main: Renderer creation failed, exiting...");
        platform_destroy(platform);
        return 1;
    }

    SDL_SetWindowRelativeMouseMode(platform_get_window(platform), true);

    bool running = true;
    while (running) {
        camera_t* camera = renderer_get_camera(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                float sensitivity = 0.1f;
                camera->yaw += event.motion.xrel * sensitivity;
                camera->pitch -= event.motion.yrel * sensitivity;

                if (camera->pitch > 89.0f) {
                    camera->pitch = 89.0f;
                }
                if (camera->pitch < -89.0f) {
                    camera->pitch = -89.0f;
                }
            }
        }

        const float cam_speed = 0.05f;
        const bool* keystate  = SDL_GetKeyboardState(NULL);

        float yaw_rad = camera->yaw * (M_PI / 180.0f);
        float cos_y   = cosf(yaw_rad);
        float sin_y   = sinf(yaw_rad);

        // Forward Vector (The direction you are looking)
        // Note how X uses SIN and Z uses COS
        float forward_x = sin_y;
        float forward_z = -cos_y;

        // Right Vector (Perpendicular to Forward)
        float right_x = cos_y;
        float right_z = sin_y;

        if (keystate[SDL_SCANCODE_W]) {
            camera->pos.x += forward_x * cam_speed;
            camera->pos.z += forward_z * cam_speed;
        }
        if (keystate[SDL_SCANCODE_S]) {
            camera->pos.x -= forward_x * cam_speed;
            camera->pos.z -= forward_z * cam_speed;
        }
        if (keystate[SDL_SCANCODE_A]) {
            camera->pos.x -= right_x * cam_speed;
            camera->pos.z -= right_z * cam_speed;
        }
        if (keystate[SDL_SCANCODE_D]) {
            camera->pos.x += right_x * cam_speed;
            camera->pos.z += right_z * cam_speed;
        }
        renderer_draw(renderer, platform_get_window(platform));
    }

    log_info("Main: Closing down...");
    renderer_destroy(renderer);
    platform_destroy(platform);
    SDL_Quit();
    return 0;
}
