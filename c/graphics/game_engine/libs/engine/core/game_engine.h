#pragma once

#include <stdbool.h>

#include "engine/core/camera.h"
#include "engine/core/scene.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"

typedef struct platform_t platform_t;
typedef struct graphics_t graphics_t;

typedef enum {
    SCENE_MAIN_MENU,
    SCENE_STATIC_LEVEL,
    SCENE_OPEN_WORLD,
} scene_type_t;

typedef struct {
    const char* window_title;
    int         window_width;
    int         window_height;
    const char* initial_pak_path;
} game_engine_init_config_t;

typedef struct {
    int   resolution_x;
    int   resolution_y;
    bool  vsync_enabled;
    float fov;
} graphic_settings_t;

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

    scene_t main_scene;

    mesh_handle_t test_mesh;

    texture_handle_t test_texture;

    draw_mode_t draw_mode;

    bool   debug_freeze_culling;
    mat4_t culling_view_proj;

    render_target_handle_t main_scene_target;

    scene_type_t active_scene_type;

    texture_handle_t skybox_texture;
    texture_handle_t skybox_irradiance_texture;
    texture_handle_t skybox_prefiltered_texture;

} game_engine_t;

bool game_engine_init(game_engine_t* game_engine, game_engine_init_config_t* engine_init_config);
bool game_engine_tick(game_engine_t* game_engine);
void game_engine_shutdown(game_engine_t* game_engine);
