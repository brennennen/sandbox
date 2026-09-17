
#include <string.h>

#include "engine/modules/graphics/debug/debug_grid.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"
#include "vk_gpu_allocator.h"
#include "vk_types.h"

void init_debug_grid(graphics_t* graphics) {
    int   grid_size = 10;
    float grid_step = 1.0f;

    graphics->grid_vertex_count = debug_grid_vertex_count(grid_size);
    size_t buffer_size          = graphics->grid_vertex_count * sizeof(vertex_t);

    graphics->grid_buffer.allocation = gpu_heap_alloc(
        graphics->assets.vertex_heap, buffer_size, 16
    );

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    // log_info("vkCreateBuffer - grid_buffer: %d", buffer_info.size);
    vkCreateBuffer(graphics->core.device, &buffer_info, NULL, &graphics->grid_buffer.buffer);
    vkBindBufferMemory(
        graphics->core.device,
        graphics->grid_buffer.buffer,
        graphics->assets.vertex_heap->memory,
        graphics->grid_buffer.allocation.offset
    );

    vertex_t* mapped_data = (vertex_t*)graphics->grid_buffer.allocation.mapped_ptr;
    generate_grid(mapped_data, grid_size, grid_step);
}

void init_debug_frustum_buffer(graphics_t* graphics) {
    size_t buffer_size                  = 24 * sizeof(vertex_t);
    graphics->frustum_buffer.allocation = gpu_heap_alloc(
        graphics->assets.vertex_heap, buffer_size, 16
    );

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    // log_info("vkCreateBuffer - frustum_buffer: %d", buffer_info.size);
    vkCreateBuffer(graphics->core.device, &buffer_info, NULL, &graphics->frustum_buffer.buffer);
    vkBindBufferMemory(
        graphics->core.device,
        graphics->frustum_buffer.buffer,
        graphics->assets.vertex_heap->memory,
        graphics->frustum_buffer.allocation.offset
    );
}

void graphics_update_debug_frustum(graphics_t* r, mat4_t inv_vp) {
    // 8 corners of Vulkan's NDC space
    vec4_t ndc[8] = {
        {-1, -1, 0, 1},
        {1, -1, 0, 1},
        {1, 1, 0, 1},
        {-1, 1, 0, 1}, // Near
        {-1, -1, 1, 1},
        {1, -1, 1, 1},
        {1, 1, 1, 1},
        {-1, 1, 1, 1} // Far
    };

    vec3_t corners[8];
    for (int i = 0; i < 8; i++) {
        vec4_t world_pos = mat4_mul_vec4(inv_vp, ndc[i]);
        corners[i]       = (vec3_t){
            world_pos.x / world_pos.w,
            world_pos.y / world_pos.w,
            world_pos.z / world_pos.w,
        };
    }

    vec4_t c = {1.0f, 1.0f, 0.0f, 1.0f}; // yellow
    // clang-format off
    vertex_t lines[24] = {
        // near face
        {.pos = corners[0], .color = c}, {.pos = corners[1], .color = c},
        {.pos = corners[1], .color = c}, {.pos = corners[2], .color = c},
        {.pos = corners[2], .color = c}, {.pos = corners[3], .color = c},
        {.pos = corners[3], .color = c}, {.pos = corners[0], .color = c},
        // far face
        {.pos = corners[4], .color = c}, {.pos = corners[5], .color = c},
        {.pos = corners[5], .color = c}, {.pos = corners[6], .color = c},
        {.pos = corners[6], .color = c}, {.pos = corners[7], .color = c},
        {.pos = corners[7], .color = c}, {.pos = corners[4], .color = c},
        // edges
        {.pos = corners[0], .color = c}, {.pos = corners[4], .color = c},
        {.pos = corners[1], .color = c}, {.pos = corners[5], .color = c},
        {.pos = corners[2], .color = c}, {.pos = corners[6], .color = c},
        {.pos = corners[3], .color = c}, {.pos = corners[7], .color = c},
    };
    // clang-format on

    memcpy(r->frustum_buffer.allocation.mapped_ptr, lines, sizeof(lines));
}
