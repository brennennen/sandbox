

#ifndef VK_PASSES_H
#define VK_PASSES_H

#include <stdint.h>

#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"

#include "vk_types.h"

void shadow_pass(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    vk_render_target_t*     shadow_render_target,
    render_stats_t*         out_render_stats
);

void forward_pass(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    vk_render_target_t*     render_target,
    uint32_t                image_index,
    render_stats_t*         out_render_stats
);

void post_process_pass(
    graphics_t*         graphics,
    vk_render_target_t* render_target,
    uint32_t            image_index
);

#endif // VK_PASSES_H
