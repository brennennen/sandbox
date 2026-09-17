
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/vec3_math.h"

#include "libs/core/resources/image.h"

#include "engine/core/game_engine.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/core/vfs.h"
#include "engine/core/world.h"
#include "engine/debug/engine_diagnostics.h"
#include "engine/debug/freefly_camera.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/assets/pak_loader.h"
#include "engine/modules/debug_imgui/debug_imgui.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/platform/platform.h"

#include "shared/math_types.h"

static bool init_core_subsystems(
    game_engine_t*             engine,
    game_engine_init_config_t* engine_init_config
) {
    engine->platform = platform_create(
        engine_init_config->window_title,
        engine_init_config->window_width,
        engine_init_config->window_height
    );
    if (!engine->platform) {
        return false;
    }

    int window_width, window_height;
    platform_get_window_size(engine->platform, &window_width, &window_height);

    graphics_config_t graphics_config = {
        .width        = window_width,
        .height       = window_height,
        .app_name     = engine_init_config->window_title,
        .present_mode = PRESENT_MODE_IMMEDIATE,
    };

    engine->graphics = graphics_create(engine->platform, &graphics_config);
    if (!engine->graphics) {
        return false;
    }

    render_target_config_t render_target_config = {
        .width          = window_width,
        .height         = window_height,
        .format         = RT_FORMAT_HDR,
        .requires_depth = true
    };
    engine->main_scene_target = graphics_create_render_target(
        engine->graphics, &render_target_config
    );

    render_target_config_t shadow_config = {
        .width          = 4096,
        .height         = 4096,
        .format         = RT_FORMAT_DEPTH_ONLY,
        .requires_depth = true,
    };
    engine->shadow_target = graphics_create_render_target(engine->graphics, &shadow_config);

    graphics_update_shadow_map_descriptor(engine->graphics, engine->shadow_target);

    platform_set_relative_mouse(engine->platform, true);
    return true;
}

static void init_default_textures(game_engine_t* engine) {
    static uint8_t white_pixel[4]  = {255, 255, 255, 255};
    image_t        dummy_white_img = {
               .width    = 1,
               .height   = 1,
               .channels = 4,
               .size     = 4,
               .pixels   = white_pixel,
    };

    engine->default_tex = graphics_upload_texture(
        engine->graphics, &dummy_white_img, PAK_TEX_FORMAT_RGBA8_SRGB
    );

    static uint8_t flat_normal_pixel[4] = {128, 128, 255, 255};
    image_t        dummy_normal_img     = {
                   .width    = 1,
                   .height   = 1,
                   .channels = 4,
                   .size     = 4,
                   .pixels   = flat_normal_pixel,
    };
    engine->default_normal_tex = graphics_upload_texture(
        engine->graphics, &dummy_normal_img, PAK_TEX_FORMAT_R8_UNORM
    );

    static uint8_t flat_mr_pixel[4]                = {255, 128, 0, 255};
    image_t        dummy_ao_metallic_roughness_img = {
               .width    = 1,
               .height   = 1,
               .channels = 4,
               .size     = 4,
               .pixels   = flat_mr_pixel,
    };
    engine->default_ao_metallic_roughness_tex = graphics_upload_texture(
        engine->graphics, &dummy_ao_metallic_roughness_img, PAK_TEX_FORMAT_RGBA8_UNORM
    );
}

bool game_engine_init(game_engine_t* game_engine, game_engine_init_config_t* engine_init_config) {
    log_info("Initializing engine...");
    if (!init_core_subsystems(game_engine, engine_init_config)) {
        log_error("Failed to initialize core subsystems.");
        return false;
    }

    debug_imgui_init(game_engine);
    platform_set_event_callback(game_engine->platform, debug_imgui_process_event);

    init_default_textures(game_engine);

    if (!vfs_mount_archive(engine_init_config->initial_pak_path)) {
        log_error("Failed to mount base game archive!");
    }

    game_engine->active_scene_type = SCENE_STATIC_LEVEL;

    load_pak_file(game_engine, engine_init_config);

    // TODO: malloc for now, camera will be part of scene, this is a debug camera
    game_engine->main_camera             = malloc(sizeof(camera_t));
    game_engine->main_camera->pos        = (vec3_t){0.0f, -5.0f, 2.0f};
    game_engine->main_camera->pitch      = -20.0f;
    game_engine->main_camera->yaw        = 0.0f;
    game_engine->main_camera->fov        = 0.785398f;
    game_engine->main_camera->near_plane = 0.1f;
    game_engine->main_camera->far_plane  = 5000.0f;

    game_engine->last_time       = platform_get_ticks(game_engine->platform);
    game_engine->fps_last_time   = game_engine->last_time;
    game_engine->fps_frame_count = 0;

    game_engine->show_debug_widgets = true;
    game_engine->is_running         = true;

    return true;
}

// Define a simple struct for grid coordinates if you don't have one
typedef struct {
    int x;
    int y;
} grid_coord_t;

static void static_level_update(game_engine_t* engine, float delta_time) {
    // todo: callback functions to game logic?
}

static void update_world_streaming(game_engine_t* engine, vec3_t player_pos) {
    // grid cell = 1000.0f world units
    int current_cell_x = (int)floorf(player_pos.x / 1000.0f);
    int current_cell_y = (int)floorf(player_pos.y / 1000.0f);

    // 3x3 grid of cells around the player
    grid_coord_t active_cells[9];
    int          idx = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            active_cells[idx++] = (grid_coord_t){current_cell_x + x, current_cell_y + y};
        }
    }

    // TODO: unload old cells

    // TODO: mount new cells
}

static bool engine_process_events(game_engine_t* engine) {
    if (!platform_update(engine->platform)) {
        return false;
    }

    if (platform_get_key_pressed(engine->platform, KEY_F3)) {
        engine->show_debug_widgets = !engine->show_debug_widgets;
        log_info("show_debug_widgets: %d", engine->show_debug_widgets);
    }

    if (platform_get_key_pressed(engine->platform, KEY_F4)) {
        return false;
    }

    if (platform_get_key_pressed(engine->platform, KEY_F5)) {
        engine->draw_mode = (engine->draw_mode + 1) % DRAW_MODE_COUNT;
        // Skip SDR, doesn't work currently.
        if (engine->draw_mode == DRAW_MODE_DEBUG_SDR) {
            engine->draw_mode = (engine->draw_mode + 1) % DRAW_MODE_COUNT;
        }
        log_info("draw mode: %s", draw_mode_names[engine->draw_mode]);
    }

    if (platform_get_key_pressed(engine->platform, KEY_F6)) {
        engine->debug_freeze_culling = !engine->debug_freeze_culling;
        if (engine->debug_freeze_culling) {
            log_info("Frustum Culling: FROZEN");
            mat4_t inv_culling = mat4_inverse(engine->culling_view_proj);
            graphics_update_debug_frustum(engine->graphics, inv_culling);
        } else {
            log_info("Frustum Culling: UNFREEZE");
        }
    }

    if (platform_get_key_pressed(engine->platform, KEY_ESCAPE)) {
        engine->is_paused = !engine->is_paused;
        if (engine->is_paused) {
            platform_set_relative_mouse(engine->platform, false);
            log_info("paused");
        } else {
            platform_set_relative_mouse(engine->platform, true);
            log_info("unpaused");
        }
    }

    if (platform_get_key_pressed(engine->platform, KEY_F12)) {
        present_mode_t current_mode = graphics_get_present_mode(engine->graphics);
        present_mode_t next_mode    = (current_mode == PRESENT_MODE_VSYNC) ? PRESENT_MODE_IMMEDIATE
                                                                           : PRESENT_MODE_VSYNC;
        graphics_set_present_mode(engine->graphics, next_mode);
        log_info("Toggled Present Mode to: %d", next_mode);
    }

    return true;
}

static float engine_update_time(game_engine_t* engine) {
    uint64_t current_time = platform_get_ticks(engine->platform);
    engine->delta_time    = (current_time - engine->last_time) / 1000.0f;
    engine->last_time     = current_time;

    if (engine->delta_time > 0.1f) {
        engine->delta_time = 0.1f;
    }

    engine->fps_frame_count++;
    if (current_time - engine->fps_last_time >= 1000) {
        engine->fps_last_1s_avg = engine->fps_frame_count;
        engine->fps_frame_count = 0;
        engine->fps_last_time   = current_time;
    }
    return engine->delta_time;
}

static void engine_update_simulation(game_engine_t* engine, float delta_time) {
    if (engine->is_paused) {
        return;
    }

    freefly_camera_update(engine->main_camera, engine->platform, delta_time);

    // TODO: add conditional support for levels, quad/cube worlds, or non-standard/custom worlds
    // (cylindrical?)
    switch (engine->active_scene_type) {
    case SCENE_MAIN_MENU: {
        // main_menu_update(engine, delta_time);
        log_error("not implemented");
        break;
    }
    case SCENE_STATIC_LEVEL: {
        static_level_update(engine, delta_time);
        break;
    }
    case SCENE_OPEN_WORLD: {
        // update_world_streaming(engine, engine->main_camera->pos);
        log_error("not implemented");
        break;
    }
    }
}

static void engine_render_frame(game_engine_t* engine) {
    mat4_t view = camera_get_view_matrix(engine->main_camera);

    int w;
    int h;
    platform_get_window_size(engine->platform, &w, &h);
    float aspect = (float)w / (float)h;

    mat4_t proj = mat4_perspective(
        engine->main_camera->fov,
        aspect,
        engine->main_camera->near_plane,
        engine->main_camera->far_plane
    );
    mat4_t current_view_proj = mat4_mul(proj, view);
    vec3_t target            = engine->main_camera->pos;
    target.z                 = 0.0f; // ignore z so the shadow box stays grounded
    float  shadow_distance   = 200.0f;
    vec3_t light_dir         = engine->environment.sun_direction;
    vec3_t offset            = {
        light_dir.x * -shadow_distance,
        light_dir.y * -shadow_distance,
        light_dir.z * -shadow_distance
    };
    vec3_t light_pos = {target.x + offset.x, target.y + offset.y, target.z + offset.z};

    vec3_t up = {0.0f, 0.0f, 1.0f};
    // If looking almost straight down (or straight up), use Y as the up vector
    if (fabsf(light_dir.x) < 0.001f && fabsf(light_dir.y) < 0.001f) {
        up = (vec3_t){0.0f, 1.0f, 0.0f};
    }
    mat4_t light_view = mat4_look_at(light_pos, target, up);
    float  ortho_size = 50.0f;
    mat4_t light_proj = mat4_ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, 1.0f, 400.0f);
    mat4_t light_space_matrix = mat4_mul(light_proj, light_view);
    // engine->environment.sun_direction = vec3_normalize(vec3_sub(target, light_pos));

    if (!engine->debug_freeze_culling) {
        engine->culling_view_proj = current_view_proj;
    }

    graphics_frame_input_t gfx_frame_input = {
        .target             = engine->main_scene_target,
        .shadow_target      = engine->shadow_target,
        .view               = view,
        .proj               = proj,
        .camera_pos         = engine->main_camera->pos,
        .culling_view_proj  = engine->culling_view_proj,
        .is_culling_frozen  = engine->debug_freeze_culling,
        .light_space_matrix = light_space_matrix,
        .draw_mode          = engine->draw_mode,
        .environment        = &engine->environment,
        .scene              = &engine->main_scene,
        .show_debug_widgets = engine->show_debug_widgets
    };
    vec3_t origin = {0.0f, 0.0f, 1.0f};

    graphics_update_debug_sun_line(engine->graphics, origin, engine->environment.sun_direction);

    graphics_draw(engine->graphics, engine->platform, &gfx_frame_input);
}

bool game_engine_tick(game_engine_t* game_engine) {
    if (!engine_process_events(game_engine)) {
        return false;
    }
    float delta_time = engine_update_time(game_engine);
    engine_update_simulation(game_engine, delta_time);

    debug_imgui_begin_frame();
    if (game_engine->show_debug_widgets) {
        engine_diagnostics_draw_panel(game_engine);
    }

    engine_render_frame(game_engine);
    return true;
}

void game_engine_shutdown(game_engine_t* game_engine) {
    log_info("Shutting down engine...");
    if (game_engine->graphics) {
        graphics_wait_idle(game_engine->graphics);
    }

    debug_imgui_shutdown(game_engine);

    if (game_engine->main_scene_target.id != GRAPHICS_INVALID_HANDLE) {
        graphics_destroy_render_target(game_engine->graphics, game_engine->main_scene_target);
    }

    if (game_engine->shadow_target.id != GRAPHICS_INVALID_HANDLE) {
        graphics_destroy_render_target(game_engine->graphics, game_engine->shadow_target);
    }

    if (game_engine->graphics) {
        graphics_destroy(game_engine->graphics);
    }

    if (game_engine->platform) {
        platform_destroy(game_engine->platform);
    }
}
