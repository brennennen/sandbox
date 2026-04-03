
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>

#include "core/logger.h"
#include "core/math/mat4_math.h"
#include "core/math/math_types.h"
#include "game_engine.h"
#include "modules/assets/gltf.h"
#include "modules/assets/obj.h"
#include "modules/graphics/graphics.h"
#include "platform/platform.h"

bool game_engine_init(game_engine_t* game_engine) {
    log_info("Initializing engine...");

    game_engine->platform = platform_create("Game Engine", 800, 600);
    if (!game_engine->platform) {
        return false;
    }

    int w;
    int h;
    platform_get_window_size(game_engine->platform, &w, &h);

    graphics_config_t graphics_config = {
        .width        = w,
        .height       = h,
        .app_name     = "Game Engine",
        .present_mode = PRESENT_MODE_IMMEDIATE, // PRESENT_MODE_VSYNC,
    };

    game_engine->graphics = graphics_create(game_engine->platform, &graphics_config);
    if (!game_engine->graphics)
        return false;

    platform_set_relative_mouse(game_engine->platform, true);

    // TODO: malloc for now, camera will be part of scene, this is a debug camera
    game_engine->main_camera        = malloc(sizeof(camera_t));
    game_engine->main_camera->pos   = (vec3_t){0.0f, -5.0f, 2.0f};
    game_engine->main_camera->pitch = -20.0f;
    game_engine->main_camera->yaw   = 0.0f;

    game_engine->last_time     = platform_get_ticks(game_engine->platform);
    game_engine->fps_last_time = game_engine->last_time;
    game_engine->frame_count   = 0;

    mesh_data_t my_model_data;

    // TODO: go back to pink/black checkerboard default texture?
    uint8_t white_pixel[4]  = {255, 255, 255, 255};
    image_t dummy_white_img = {
        .width    = 1,
        .height   = 1,
        .channels = 4,
        .size     = 4,
        .pixels   = white_pixel,
    };
    texture_handle_t default_tex = graphics_upload_texture(game_engine->graphics, &dummy_white_img);

    if (!load_gltf_scene(
            "test_glb.glb",
            &game_engine->main_scene,
            game_engine->graphics,
            game_engine->test_texture
        )) {
        log_error("Failed to load complex GLTF scene");
    }

    game_engine->is_running = true;
    return true;
}

static void game_engine_handle_inputs(game_engine_t* game_engine, float delta_time) {
    camera_t*   camera   = game_engine->main_camera;
    platform_t* platform = game_engine->platform;

    float dx;
    float dy;

    platform_get_mouse_delta(platform, &dx, &dy);

    float sensitivity = 0.1f;
    camera->yaw += dx * sensitivity;
    camera->pitch -= dy * sensitivity;

    if (camera->pitch > 89.0f) {
        camera->pitch = 89.0f;
    }
    if (camera->pitch < -89.0f) {
        camera->pitch = -89.0f;
    }

    const float base_cam_speed = 5.0f;
    float       cam_speed      = base_cam_speed * delta_time;

    float yaw_rad   = camera->yaw * (M_PI / 180.0f);
    float forward_x = sinf(yaw_rad);
    float forward_y = cosf(yaw_rad);
    float right_x   = cosf(yaw_rad);
    float right_y   = -sinf(yaw_rad);

    if (platform_get_key(platform, KEY_W)) {
        camera->pos.x += forward_x * cam_speed;
        camera->pos.y += forward_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_S)) {
        camera->pos.x -= forward_x * cam_speed;
        camera->pos.y -= forward_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_A)) {
        camera->pos.x -= right_x * cam_speed;
        camera->pos.y -= right_y * cam_speed;
    }
    if (platform_get_key(platform, KEY_D)) {
        camera->pos.x += right_x * cam_speed;
        camera->pos.y += right_y * cam_speed;
    }

    if (platform_get_key(platform, KEY_SPACE))
        camera->pos.z += cam_speed;
    if (platform_get_key(platform, KEY_LSHIFT))
        camera->pos.z -= cam_speed;
}

bool game_engine_tick(game_engine_t* game_engine) {
    if (!platform_update(game_engine->platform)) {
        return false;
    }
    if (platform_get_key_pressed(game_engine->platform, KEY_F4)) {
        return false;
    }
    if (platform_get_key_pressed(game_engine->platform, KEY_F5)) {
        game_engine->draw_mode = (game_engine->draw_mode + 1) % DRAW_MODE_COUNT;
        log_info("draw mode: %s", draw_mode_names[game_engine->draw_mode]);
    }
    if (platform_get_key_pressed(game_engine->platform, KEY_ESCAPE)) {
        game_engine->is_paused = !game_engine->is_paused;
        if (game_engine->is_paused) {
            platform_set_relative_mouse(game_engine->platform, false);
            log_info("paused");
        } else {
            platform_set_relative_mouse(game_engine->platform, true);
            log_info("unpaused");
        }
    }
    if (platform_get_key_pressed(game_engine->platform, KEY_F12)) {
        present_mode_t current_mode = graphics_get_present_mode(game_engine->graphics);
        present_mode_t next_mode    = (current_mode == PRESENT_MODE_VSYNC) ? PRESENT_MODE_IMMEDIATE
                                                                           : PRESENT_MODE_VSYNC;
        graphics_set_present_mode(game_engine->graphics, next_mode);
        log_info("Toggled Present Mode to: %d", next_mode);
    }

    uint64_t current_time  = platform_get_ticks(game_engine->platform);
    float    delta_time    = (current_time - game_engine->last_time) / 1000.0f;
    game_engine->last_time = current_time;

    if (delta_time > 0.1f) {
        delta_time = 0.1f;
    }

    game_engine->frame_count++;

    if (current_time - game_engine->fps_last_time >= 1000) {
        char title[128];
        snprintf(
            title,
            sizeof(title),
            "Game Engine | FPS: %d | dt: %.4fs",
            game_engine->frame_count,
            delta_time
        );
        platform_set_title(game_engine->platform, title);

        game_engine->frame_count   = 0;
        game_engine->fps_last_time = current_time;
    }

    if (!game_engine->is_paused) {
        game_engine_handle_inputs(game_engine, delta_time);
    }

    mat4_t view = camera_get_view_matrix(game_engine->main_camera);
    graphics_draw(
        game_engine->graphics,
        game_engine->platform,
        view,
        game_engine->draw_mode,
        game_engine->main_scene.objects,
        game_engine->main_scene.object_count
    );

    return true;
}

void game_engine_shutdown(game_engine_t* game_engine) {
    log_info("Shutting down engine...");
    if (game_engine->graphics) {
        graphics_destroy(game_engine->graphics);
    }
    if (game_engine->platform) {
        platform_destroy(game_engine->platform);
    }
}
