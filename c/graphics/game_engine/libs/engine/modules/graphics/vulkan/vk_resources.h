#ifndef VK_RESOURCES_H
#define VK_RESOURCES_H

#include "engine/modules/assets/image.h"
#include "shared/scene_types.h"

#include "vk_types.h"

mat4_t mat4_identity();
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
mat4_t mat4_mul(mat4_t a, mat4_t b);

gpu_allocation_t vk_create_staging_buffer(
    graphics_t*  r,
    void*        data,
    VkDeviceSize size,
    VkBuffer*    out_buffer
);

bool vk_create_texture(
    graphics_t*          r,
    image_t*             img,
    vk_texture_t*        out_tex,
    pak_texture_format_t format
);

void vk_transition_image_layout(
    graphics_t*   r,
    VkImage       image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void vk_copy_buffer_to_image(
    graphics_t* r,
    VkBuffer    buffer,
    VkImage     image,
    uint32_t    width,
    uint32_t    height
);

VkBuffer vk_create_static_buffer(
    graphics_t*        r,
    void*              data,
    VkDeviceSize       size,
    VkBufferUsageFlags usage
);

#endif