#ifndef VK_RENDER_QUEUE_H
#define VK_RENDER_QUEUE_H

#include <stdint.h>

#include "shared/math_types.h"
#include "vk_types.h"

typedef enum {
    QUEUE_OPAQUE                    = 0,
    QUEUE_OPAQUE_DOUBLE_SIDED       = 1,
    QUEUE_ALPHA_MASKED              = 2,
    QUEUE_ALPHA_MASKED_DOUBLE_SIDED = 3,
    QUEUE_UNLIT                     = 4,
    QUEUE_TRANSLUCENT               = 5,
    QUEUE_MAX_COUNT
} render_queue_id_t;

typedef struct {
    uint64_t sort_key;
    uint32_t object_index;
} render_command_t;

/**
 * @brief Evaluates a material to determine which render queue it belongs to.
 */
render_queue_id_t vk_evaluate_material_queue(const vk_material_t* mat);

int render_command_compare(const void* a, const void* b);

/**
 * @brief Generates a 64-bit sort key for the render queue.
 *
 * The key is packed to ensure optimal state binding and early-Z culling:
 * [16 bits Pipeline ID] | [16 bits Material ID] | [32 bits Depth]
 *
 * - Pipeline ID (48 - 63): Groups draws by pipeline to minimize expensive Vulkan pipeline binds.
 * - Material ID (31 - 47):  Groups draws by material to minimize descriptor set binds.
 * - Depth       (0 - 31):  Sorts objects front-to-back using squared distance to maximize
 *                          early-Z fragment rejection on the GPU.
 */
uint64_t vk_generate_sort_key(
    render_queue_id_t queue_id,
    uint16_t          material_id,
    vec3_t            camera_pos,
    vec3_t            object_pos
);

uint32_t build_render_queue(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_command_t*       out_queue,
    uint32_t                max_commands
);

void build_pipeline_map(graphics_t* graphics, draw_mode_t draw_mode, VkPipeline* out_pipeline_map);

void execute_render_queue(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_command_t*       render_queue,
    uint32_t                command_count,
    VkPipeline*             pipeline_map,
    render_stats_t*         out_render_stats
);

#endif // VK_RENDER_QUEUE_H
