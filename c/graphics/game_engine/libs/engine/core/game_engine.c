
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/core/resources/image.h"

#include "engine/core/game_engine.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/core/vfs.h"
#include "engine/core/world.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/platform/platform.h"
#include "shared/math_types.h"

static texture_handle_t default_tex;
static texture_handle_t default_normal_tex;
static texture_handle_t default_ao_metallic_roughness_tex;

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
    default_tex = graphics_upload_texture(
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
    default_normal_tex = graphics_upload_texture(
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
    default_ao_metallic_roughness_tex = graphics_upload_texture(
        engine->graphics, &dummy_ao_metallic_roughness_img, PAK_TEX_FORMAT_RGBA8_UNORM
    );
}

static void load_geometry_from_pak(game_engine_t* engine, world_pak_t* header, void* raw_pak_data) {
    log_info("Uploading raw PAK data to Vulkan...");

    pak_vertex_t* loaded_vertices = (pak_vertex_t*)((uint8_t*)raw_pak_data + header->vertex_offset);
    uint32_t*     loaded_indices  = (uint32_t*)((uint8_t*)raw_pak_data + header->index_offset);
    pak_mesh_t*   loaded_meshes   = (pak_mesh_t*)((uint8_t*)raw_pak_data + header->mesh_offset);
    pak_entity_t* loaded_entities = (pak_entity_t*)((uint8_t*)raw_pak_data + header->entity_offset);
    pak_texture_t* loaded_textures = (pak_texture_t*)((uint8_t*)raw_pak_data +
                                                      header->texture_offset);

    texture_handle_t gpu_textures[1024];
    for (uint32_t t = 0; t < header->texture_count; t++) {
        pak_texture_t* tex_def = &loaded_textures[t];
        image_t        img     = {
                       .width      = tex_def->width,
                       .height     = tex_def->height,
                       .channels   = tex_def->channels,
                       .size       = tex_def->byte_size,
                       .pixels     = (uint8_t*)raw_pak_data + tex_def->byte_offset,
                       .mip_levels = tex_def->mip_levels,
                       .is_cubemap = false
        };

        if (img.size == 0) {
            gpu_textures[t] = default_tex;
            continue;
        }
        gpu_textures[t] = graphics_upload_texture(engine->graphics, &img, tex_def->format);
    }

    for (uint32_t i = 0; i < header->mesh_count; i++) {
        pak_mesh_t* mesh_def = &loaded_meshes[i];

        mesh_data_t raw_mesh_data = {
            .vertices        = (vertex_t*)&loaded_vertices[mesh_def->vertex_offset],
            .vertex_count    = mesh_def->vertex_count,
            .indices         = &loaded_indices[mesh_def->index_offset],
            .index_count     = mesh_def->index_count,
            .bounding_center = mesh_def->bounding_center,
            .bounding_radius = mesh_def->bounding_radius,
        };

        mesh_handle_t vram_handle = graphics_upload_mesh(engine->graphics, &raw_mesh_data);

        texture_handle_t mesh_tex = (mesh_def->base_color_texture_id >= 0)
                                        ? gpu_textures[mesh_def->base_color_texture_id]
                                        : default_tex;

        texture_handle_t norm_tex = (mesh_def->normal_texture_id >= 0)
                                        ? gpu_textures[mesh_def->normal_texture_id]
                                        : default_normal_tex;

        texture_handle_t ao_mr_tex = (mesh_def->ao_roughness_metallic_texture_id >= 0)
                                         ? gpu_textures[mesh_def->ao_roughness_metallic_texture_id]
                                         : default_ao_metallic_roughness_tex;

        mat4_t final_transform = mat4_identity();
        for (uint32_t e = 0; e < header->entity_count; e++) {
            if (loaded_entities[e].model_id == mesh_def->model_id) {
                final_transform = loaded_entities[e].transform;
                break;
            }
        }

        uint32_t obj_idx                              = engine->main_scene.object_count++;
        engine->main_scene.objects[obj_idx].mesh      = vram_handle;
        engine->main_scene.objects[obj_idx].transform = final_transform;
        engine->main_scene.objects[obj_idx].material  = graphics_create_material(
            engine->graphics,
            mesh_tex,
            norm_tex,
            ao_mr_tex,
            mesh_def->is_alpha_masked,
            mesh_def->metallic_factor,
            mesh_def->roughness_factor
        );
    }
}

bool game_engine_init(game_engine_t* game_engine, game_engine_init_config_t* engine_init_config) {
    log_info("Initializing engine...");
    if (!init_core_subsystems(game_engine, engine_init_config)) {
        log_error("Failed to initialize core subsystems.");
        return false;
    }
    init_default_textures(game_engine);

    if (!vfs_mount_archive(engine_init_config->initial_pak_path)) {
        log_error("Failed to mount base game archive!");
    }

    game_engine->active_scene_type = SCENE_STATIC_LEVEL;

    void* raw_pak_data = vfs_get_mounted_archive_pointer(engine_init_config->initial_pak_path);
    if (raw_pak_data) {
        world_pak_t*   header     = (world_pak_t*)raw_pak_data;
        texture_pak_t* skybox_def = &header->environment.skybox_cubemap;

        if (skybox_def->data_size > 0) {
            void*   pixel_data = (uint8_t*)raw_pak_data + skybox_def->data_offset;
            image_t skybox_img = {
                .width      = skybox_def->width,
                .height     = skybox_def->height,
                .channels   = skybox_def->channels,
                .size       = skybox_def->data_size,
                .pixels     = pixel_data,
                .is_cubemap = true,
            };
            game_engine->skybox_texture = graphics_upload_texture(
                game_engine->graphics, &skybox_img, PAK_TEX_FORMAT_RGBA32F
            );

            texture_pak_t* irr_def = &header->environment.irradiance_map;
            image_t        irr_img = {
                       .width      = irr_def->width,
                       .height     = irr_def->height,
                       .channels   = irr_def->channels,
                       .size       = irr_def->data_size,
                       .pixels     = (uint8_t*)raw_pak_data + irr_def->data_offset,
                       .is_cubemap = true,
            };
            texture_handle_t irr_tex = graphics_upload_texture(
                game_engine->graphics, &irr_img, PAK_TEX_FORMAT_RGBA32F
            );

            texture_pak_t* pref_def = &header->environment.prefiltered_env_map;
            image_t        pref_img = {
                       .width      = pref_def->width,
                       .height     = pref_def->height,
                       .channels   = pref_def->channels,
                       .size       = pref_def->data_size,
                       .pixels     = (uint8_t*)raw_pak_data + pref_def->data_offset,
                       .mip_levels = 5,
                       .is_cubemap = true,
            };
            texture_handle_t pref_tex = graphics_upload_texture(
                game_engine->graphics, &pref_img, PAK_TEX_FORMAT_RGBA32F
            );

            // graphics_update_global_environment(
            //     game_engine->graphics, game_engine->skybox_texture, irr_tex, pref_tex
            // );
            graphics_update_global_environment(
                game_engine->graphics,
                game_engine->skybox_texture,
                game_engine->skybox_texture, // irr_tex
                game_engine->skybox_texture  // pref_tex
            );
            log_info("Successfully uploaded HDRI Skybox to GPU!");

            load_geometry_from_pak(game_engine, header, raw_pak_data);
        } else {
            game_engine->skybox_texture = default_tex; // Fallback
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
    if (platform_get_key_pressed(game_engine->platform, KEY_F6)) {
        game_engine->debug_freeze_culling = !game_engine->debug_freeze_culling;
        if (game_engine->debug_freeze_culling) {
            log_info("Frustum Culling: FROZEN");
            mat4_t inv_culling = mat4_inverse(game_engine->culling_view_proj);
            graphics_update_debug_frustum(game_engine->graphics, inv_culling);
        } else {
            log_info("Frustum Culling: UNFREEZE");
        }
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

    // TODO: add conditional support for levels, quad/cube worlds, or non-standard/custom worlds
    // (cylindrical?)
    switch (game_engine->active_scene_type) {
    case SCENE_MAIN_MENU: {
        // main_menu_update(game_engine, delta_time);
        log_error("not implemented");
        break;
    }
    case SCENE_STATIC_LEVEL: {
        static_level_update(game_engine, delta_time);
        break;
    }
    case SCENE_OPEN_WORLD: {
        // update_world_streaming(game_engine, game_engine->main_camera->pos);
        log_error("not implemented");
        break;
    }
    }

    mat4_t view = camera_get_view_matrix(game_engine->main_camera);

    int w;
    int h;
    platform_get_window_size(game_engine->platform, &w, &h);
    float aspect = (float)w / (float)h;

    mat4_t proj              = mat4_perspective(0.785f, aspect, 0.1f, 5000.0f);
    mat4_t current_view_proj = mat4_mul(proj, view);

    if (!game_engine->debug_freeze_culling) {
        game_engine->culling_view_proj = current_view_proj;
    }

    graphics_draw(
        game_engine->graphics,
        game_engine->platform,
        game_engine->main_scene_target,
        view,
        game_engine->main_camera->pos,
        game_engine->culling_view_proj,
        game_engine->debug_freeze_culling,
        game_engine->draw_mode,
        game_engine->main_scene.objects,
        game_engine->main_scene.object_count,
        game_engine->skybox_texture
    );

    return true;
}

void game_engine_shutdown(game_engine_t* game_engine) {
    log_info("Shutting down engine...");
    if (game_engine->graphics) {
        graphics_wait_idle(game_engine->graphics);
    }

    if (game_engine->main_scene_target.id != GRAPHICS_INVALID_HANDLE) {
        graphics_destroy_render_target(game_engine->graphics, game_engine->main_scene_target);
    }

    if (game_engine->graphics) {
        graphics_destroy(game_engine->graphics);
    }

    if (game_engine->platform) {
        platform_destroy(game_engine->platform);
    }
}
