#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "engine/core/camera.h"
#include "engine/modules/assets/image.h"
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

/**
 * What pipeline and shaders to use.
 */
typedef enum {
    DRAW_MODE_LIT,                   // standard rendering
    DRAW_MODE_DEBUG_WIREFRAME,       // only render edges, no faces
    DRAW_MODE_DEBUG_LIGHTING,        // render faces with lighting information, but no albedo
    DRAW_MODE_DEBUG_ALBEDO,          // render faces unlit with albedo
    DRAW_MODE_DEBUG_GEOMETRY_NORMAL, // render faces with geometry normal as face color
    DRAW_MODE_DEBUG_TEXTURE_NORMAL,  // render faces with texture normal as face color
    DRAW_MODE_DEBUG_NORMAL,          // combined normal
    DRAW_MODE_DEBUG_TANGENT,
    DRAW_MODE_DEBUG_BITANGENT,
    DRAW_MODE_DEBUG_VERTEX_COLOR, // render the vertex color

    DRAW_MODE_COUNT,
} draw_mode_t;
extern const char* const draw_mode_names[];

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
    texture_handle_t normal
);

void graphics_draw(
    graphics_t*      graphics,
    platform_t*      platform,
    mat4_t           view,
    draw_mode_t      draw_mode,
    render_object_t* objects,
    uint32_t         object_count
);

void graphics_destroy(graphics_t* graphics);

#endif
