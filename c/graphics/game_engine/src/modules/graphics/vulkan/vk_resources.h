#ifndef VK_RESOURCES_H
#define VK_RESOURCES_H

#include "modules/assets/image.h"
#include "vk_types.h"

mat4_t mat4_identity();
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
mat4_t mat4_mul(mat4_t a, mat4_t b);

gpu_allocation_t vk_create_staging_buffer(
    renderer_t*  r,
    void*        data,
    VkDeviceSize size,
    VkBuffer*    out_buffer
);

VkCommandBuffer vk_begin_single_time_commands(renderer_t* r);

void vk_end_single_time_commands(renderer_t* r, VkCommandBuffer cmd);

bool vk_create_dummy_texture(renderer_t* r);

bool vk_create_texture(renderer_t* r, image_t* img);

void vk_transition_image_layout(
    renderer_t*   r,
    VkImage       image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void vk_copy_buffer_to_image(
    renderer_t* r,
    VkBuffer    buffer,
    VkImage     image,
    uint32_t    width,
    uint32_t    height
);

VkBuffer vk_create_static_buffer(
    renderer_t*        r,
    void*              data,
    VkDeviceSize       size,
    VkBufferUsageFlags usage
);

void vk_transition_depth_layout(renderer_t* r, VkImage image);

bool vk_setup_depth_buffer(renderer_t* r, uint32_t width, uint32_t height);

#endif