#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "core/camera.h"
#include "core/math/math_types.h"
#include "modules/assets/image.h"
#include "modules/graphics/graphics_types.h"
#include "platform/platform.h"

typedef struct graphics_t graphics_t;
typedef struct platform_t platform_t;

graphics_t* graphics_create(platform_t* platform, int width, int height);

void graphics_destroy(graphics_t* graphics);

mesh_handle_t graphics_upload_mesh(graphics_t* graphics, mesh_data_t* img);

texture_handle_t graphics_upload_texture(graphics_t* graphics, image_t* data);

void graphics_draw(
    graphics_t*      graphics,
    platform_t*      platform,
    mat4_t           view,
    render_object_t* objects,
    uint32_t         object_count
);

#endif
