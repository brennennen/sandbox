#pragma once

#include "libs/core/resources/image.h"

#include "engine/core/camera.h"
#include "engine/modules/graphics/graphics_types.h"
#include "engine/platform/platform.h"
#include "shared/math_types.h"
#include "shared/scene_types.h"

typedef struct graphics_t graphics_t;
typedef struct platform_t platform_t;

/**
 * How the engine synchronizes rendered frames with the monitor.
 */
typedef enum {
    /** Uncapped FPS. Pushes images to the screen immediately. Lowest possible latency at the cost
       of potential screen tearing.*/
    PRESENT_MODE_IMMEDIATE,
    /** Uncapped FPS, but waits for the monitor's refresh to display. Low latency and no tearing,
       but the GPU wastes power rendering unseen frames.*/
    PRESENT_MODE_MAILBOX,
    /** Capped FPS (V-Sync). Locks to the monitor's refresh rate (60hz, 144hz,
     etc.). No screen tearing at the cost of higher latency. */
    PRESENT_MODE_VSYNC,
} present_mode_t;

typedef struct {
    int            width;
    int            height;
    const char*    app_name;
    present_mode_t present_mode;
} graphics_config_t;

graphics_t* graphics_create(platform_t* platform, graphics_config_t* config);

present_mode_t graphics_get_present_mode(const graphics_t* graphics);

void graphics_set_present_mode(graphics_t* graphics, present_mode_t mode);

mesh_handle_t graphics_upload_mesh(graphics_t* graphics, mesh_data_t* img);

texture_handle_t graphics_upload_texture(graphics_t* r, image_t* img, pak_texture_format_t format);

material_handle_t graphics_create_material(
    graphics_t*      r,
    texture_handle_t albedo,
    texture_handle_t normal,
    texture_handle_t metallic_roughness,
    bool             is_alpha_masked,
    float            metallic_factor,
    float            roughness_factor
);

render_target_handle_t graphics_create_render_target(
    graphics_t*                   graphics,
    const render_target_config_t* render_target_config
);

void graphics_destroy_render_target(graphics_t* graphics, render_target_handle_t handle);

void graphics_update_shadow_map_descriptor(
    graphics_t*            graphics,
    render_target_handle_t shadow_target
);

void graphics_draw(
    graphics_t*             graphics,
    platform_t*             platform,
    graphics_frame_input_t* gfx_frame_input
);

void graphics_destroy(graphics_t* graphics);

void graphics_update_debug_frustum(graphics_t* r, mat4_t inv_vp);

void graphics_wait_idle(graphics_t* graphics);

void graphics_update_global_environment(
    graphics_t*      graphics,
    texture_handle_t skybox_tex,
    texture_handle_t irradiance_tex,
    texture_handle_t prefiltered_tex
);
