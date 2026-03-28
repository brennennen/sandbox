#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "core/camera.h"
#include "core/math/math_types.h"
#include "modules/assets/image.h"
#include "modules/graphics/graphics_types.h"
#include "platform/platform.h"

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

texture_handle_t graphics_upload_texture(graphics_t* graphics, image_t* data);

void graphics_draw(
    graphics_t*      graphics,
    platform_t*      platform,
    mat4_t           view,
    render_object_t* objects,
    uint32_t         object_count
);

void graphics_destroy(graphics_t* graphics);

#endif
