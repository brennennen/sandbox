#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <stdbool.h>

#include "core/camera.h"

typedef struct platform_t platform_t;
typedef struct renderer_t renderer_t;

typedef struct {
    bool is_running;

    platform_t* platform;
    renderer_t* renderer;
    // physics_t* physics;
    // audio_engine_t* audio;

    // High-level Gameplay State
    camera_t* main_camera;
    // scene_t* active_scene;

    uint64_t last_time;

} game_engine_t;

bool game_engine_init(game_engine_t* game_engine);
bool game_engine_tick(game_engine_t* game_engine);
void game_engine_shutdown(game_engine_t* game_engine);

#endif // GAME_ENGINE_H
