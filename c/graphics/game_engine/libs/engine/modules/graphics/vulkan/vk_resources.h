#pragma once

#include "libs/core/resources/image.h"

#include "shared/scene_types.h"

#include "scene_types.h"
#include "vk_types.h"

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
    VkImageLayout new_layout,
    uint32_t      mip_levels,
    bool          is_cubemap
);

void vk_copy_buffer_to_image(
    graphics_t*          r,
    VkBuffer             buffer,
    VkImage              image,
    uint32_t             width,
    uint32_t             height,
    uint32_t             mip_levels,
    pak_texture_format_t format,
    bool                 is_cubemap
);

VkBuffer vk_create_static_buffer(
    graphics_t*        r,
    void*              data,
    VkDeviceSize       size,
    VkBufferUsageFlags usage
);
