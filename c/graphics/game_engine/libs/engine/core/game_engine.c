
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>

#include "engine/core/game_engine.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
#include "engine/core/vfs.h"
#include "engine/core/world.h"
#include "engine/modules/assets/gltf.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/platform/platform.h"
#include "shared/math_types.h"

static texture_handle_t default_tex;

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
        .present_mode = PRESENT_MODE_IMMEDIATE,
    };

    game_engine->graphics = graphics_create(game_engine->platform, &graphics_config);
    if (!game_engine->graphics)
        return false;

    platform_set_relative_mouse(game_engine->platform, true);

    // TODO: go back to pink/black checkerboard default texture?
    static uint8_t white_pixel[4] = {255, 255, 255, 255};

    image_t dummy_white_img = {
        .width    = 1,
        .height   = 1,
        .channels = 4,
        .size     = 4,
        .pixels   = white_pixel,
    };
    default_tex = graphics_upload_texture(
        game_engine->graphics, &dummy_white_img, PAK_TEX_FORMAT_RGBA8_SRGB
    );

    static uint8_t flat_normal_pixel[4] = {128, 128, 255, 255};
    image_t        dummy_normal_img     = {
                   .width    = 1,
                   .height   = 1,
                   .channels = 4,
                   .size     = 4,
                   .pixels   = flat_normal_pixel,
    };
    texture_handle_t default_normal_tex = graphics_upload_texture(
        game_engine->graphics, &dummy_normal_img, PAK_TEX_FORMAT_R8_UNORM
    );

    if (!vfs_mount_archive("./../../.assets/test_zone.pak")) {
        log_error("Failed to mount base game archive!");
    }

    if (world_load_chunk(0)) {
        log_info("Uploading raw PAK data to Vulkan...");

        texture_handle_t gpu_textures[MAX_LOADED_TEXTURES];

        for (uint32_t t = 0; t < loaded_texture_count; t++) {
            image_t img = {0};

            pak_texture_format_t pak_format = loaded_textures[t].format;

            if (world_load_texture_image(t, &img)) {
                gpu_textures[t] = graphics_upload_texture(game_engine->graphics, &img, pak_format);
                image_free(&img);
            } else {
                gpu_textures[t] = default_tex;
            }
        }

        for (uint32_t i = 0; i < loaded_mesh_count; i++) {
            pak_mesh_t* mesh_def = &loaded_meshes[i];

            vertex_t* vertex_start = &loaded_vertices[mesh_def->vertex_offset];
            uint32_t* index_start  = &loaded_indices[mesh_def->index_offset];

            mesh_data_t raw_mesh_data = {
                .vertices     = vertex_start,
                .vertex_count = mesh_def->vertex_count,
                .indices      = index_start,
                .index_count  = mesh_def->index_count
            };

            mesh_handle_t vram_handle = graphics_upload_mesh(game_engine->graphics, &raw_mesh_data);

            texture_handle_t mesh_tex = default_tex;
            if (mesh_def->base_color_texture_id >= 0 &&
                mesh_def->base_color_texture_id < (int32_t)loaded_texture_count) {
                mesh_tex = gpu_textures[mesh_def->base_color_texture_id];
            }
            texture_handle_t norm_tex = default_normal_tex;
            if (mesh_def->normal_texture_id >= 0 &&
                mesh_def->normal_texture_id < (int32_t)loaded_texture_count) {
                norm_tex = gpu_textures[mesh_def->normal_texture_id];
            }

            if (mesh_def->base_color_texture_id >= 0 &&
                mesh_def->base_color_texture_id < (int32_t)loaded_texture_count) {
                mesh_tex = gpu_textures[mesh_def->base_color_texture_id];
            }

            mat4_t final_transform = mat4_identity();
            for (uint32_t e = 0; e < loaded_entity_count; e++) {
                if (loaded_entities[e].model_id == mesh_def->model_id) {
                    final_transform = loaded_entities[e].transform;
                    break;
                }
            }

            uint32_t obj_idx = game_engine->main_scene.object_count++;

            game_engine->main_scene.objects[obj_idx].mesh     = vram_handle;
            game_engine->main_scene.objects[obj_idx].material = graphics_create_material(
                game_engine->graphics, mesh_tex, norm_tex
            );
            game_engine->main_scene.objects[obj_idx].transform = final_transform;
        }
    }

    // TODO: malloc for now, camera will be part of scene, this is a debug camera
    game_engine->main_camera        = malloc(sizeof(camera_t));
    game_engine->main_camera->pos   = (vec3_t){0.0f, -5.0f, 2.0f};
    game_engine->main_camera->pitch = -20.0f;
    game_engine->main_camera->yaw   = 0.0f;

    game_engine->last_time     = platform_get_ticks(game_engine->platform);
    game_engine->fps_last_time = game_engine->last_time;
    game_engine->frame_count   = 0;

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
