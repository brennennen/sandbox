#pragma once

#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"

#include "vk_types.h"

render_target_handle_t graphics_create_render_target(
    graphics_t*                   graphics,
    const render_target_config_t* render_target_config
);

void vk_destroy_render_target(graphics_t* graphics, render_target_handle_t render_target);

void vk_resize_render_target(
    graphics_t*         graphics,
    vk_render_target_t* rt,
    uint32_t            width,
    uint32_t            height
);

// void vk_cmd_transition_target_for_read()
