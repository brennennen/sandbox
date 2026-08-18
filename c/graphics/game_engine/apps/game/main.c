
#include <stdio.h>

#include <math.h>

#include <SDL3/SDL.h>

#include "libs/engine/core/game_engine.h"
#include "libs/engine/core/logger.h"

int main(int argc, char* argv[]) {
    log_init();

    graphic_settings_t graphic_settings = {
        .resolution_x  = 1280,
        .resolution_y  = 720,
        .vsync_enabled = false,
        .fov           = 90.0f,
    };
    // TODO: load graphic_settings from file
    // TODO: validate graphic settings

    game_engine_init_config_t engine_init_config = {
        .window_title     = "Game",
        .window_width     = graphic_settings.resolution_x,
        .window_height    = graphic_settings.resolution_y,
        .initial_pak_path = "./../../.assets/sponza.pak",
        //.initial_pak_path = "./../../.assets/test_zone.pak",
        // .initial_pak_path = "./../../.assets/test_skybox_zone.pak",
        // .initial_pak_path = "./../../.assets/render_tests.pak",
    };
    game_engine_t game_engine = {0};

    if (game_engine_init(&game_engine, &engine_init_config)) {
        bool running = true;
        while (running) {
            running = game_engine_tick(&game_engine);
        }
    } else {
        log_error("Failed to initialize game_engine.");
    }

    game_engine_shutdown(&game_engine);
    return 0;
}
