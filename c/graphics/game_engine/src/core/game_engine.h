#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <stdbool.h>

#include "core/camera.h"

#include "modules/graphics/graphics_types.h"

typedef struct platform_t platform_t;
typedef struct graphics_t graphics_t;

typedef struct {
    bool is_running;
    bool is_paused;

    platform_t* platform;
    graphics_t* graphics;
    // physics_t* physics;
    // audio_engine_t* audio;

    // High-level Gameplay State
    camera_t* main_camera;
    // scene_t* active_scene;

    uint64_t last_time;

    int      frame_count;
    uint64_t fps_last_time;

    mesh_handle_t test_mesh;

    texture_handle_t test_texture;

} game_engine_t;

bool game_engine_init(game_engine_t* game_engine);
bool game_engine_tick(game_engine_t* game_engine);
void game_engine_shutdown(game_engine_t* game_engine);

#endif // GAME_ENGINE_H
