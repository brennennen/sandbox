
#include <stdio.h>

#include <math.h>

#include <SDL3/SDL.h>

#include "libs/engine/core/game_engine.h"
#include "libs/engine/core/logger.h"

int main(int argc, char* argv[]) {
    log_init();

    // TODO: load graphic_settings from file
    // TODO: validate graphic settings

    game_engine_init_config_t engine_init_config = {
        .window_title  = "Game",
        .window_width  = 1280,
        .window_height = 720,

        .initial_pak_path = "./../../.assets/sponza_with_curtains.pak",
        //.initial_pak_path = "./../../.assets/sponza.pak",
        //.initial_pak_path = "./../../.assets/test_zone.pak",
        //.initial_pak_path = "./../../.assets/test_skybox_zone.pak",
        //.initial_pak_path = "./../../.assets/render_tests.pak",

        .fps_cap = 0,
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
