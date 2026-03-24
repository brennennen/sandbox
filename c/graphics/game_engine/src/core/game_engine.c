
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

    game_engine->graphics = graphics_create(game_engine->platform, w, h);
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

    mesh_data_t suzanne_data;
    if (load_obj("suzanne.obj", &suzanne_data)) {
        game_engine->test_mesh = graphics_upload_mesh(game_engine->graphics, &suzanne_data);
        free_mesh(&suzanne_data);
    } else {
        log_error("Failed to load suzanne.obj");
        game_engine->test_mesh.id = UINT32_MAX;
    }

    image_t img;
    if (image_load("test.png", &img)) {
        game_engine->test_texture = graphics_upload_texture(game_engine->graphics, &img);
        image_free(&img);
    } else {
        log_warn("Failed to load test.png, generating dummy texture.");
        image_t dummy             = image_create_placeholder();
        game_engine->test_texture = graphics_upload_texture(game_engine->graphics, &dummy);
        image_free(&dummy);
    }

    game_engine->is_running = true;
    return true;
}

static void game_engine_handle_inputs(game_engine_t* game_engine) {
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

    const float cam_speed = 0.05f;

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

    if (platform_get_key(platform, KEY_P)) {
        printf("Cam Pos: %.2f, %.2f, %.2f\n", camera->pos.x, camera->pos.y, camera->pos.z);
    }
}

bool game_engine_tick(game_engine_t* game_engine) {
    if (!platform_update(game_engine->platform)) {
        return false;
    }
    if (platform_get_key(game_engine->platform, KEY_ESCAPE)) {
        return false;
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

    game_engine_handle_inputs(game_engine);

    mat4_t view = camera_get_view_matrix(game_engine->main_camera);

    float time = (float)current_time / 1000.0f;

    render_object_t scene_objects[1];

    mat4_t rz = mat4_rotate_z(time * 0.5f);
    mat4_t t  = mat4_translate((vec3_t){0.0f, 0.0f, 1.0f});

    scene_objects[0].mesh      = game_engine->test_mesh;
    scene_objects[0].texture   = game_engine->test_texture;
    scene_objects[0].transform = mat4_mul(rz, t);

    graphics_draw(game_engine->graphics, game_engine->platform, view, scene_objects, 1);

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
