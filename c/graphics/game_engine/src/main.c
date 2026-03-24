
#include <math.h>

#include <SDL3/SDL.h>

#include "core/game_engine.h"
#include "core/logger.h"



int main(int argc, char* argv[]) {
    log_init();

    game_engine_t game_engine = {0};

    if (game_engine_init(&game_engine)) {
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
