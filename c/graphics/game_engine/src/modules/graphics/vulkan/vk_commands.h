#ifndef VK_COMMANDS_H
#define VK_COMMANDS_H

#include "vk_types.h"

bool            vk_create_commands(graphics_t* r);
void            vk_destroy_commands(graphics_t* r);
VkCommandBuffer vk_begin_immediate_submit(graphics_t* r);
void            vk_end_immediate_submit(graphics_t* r);
void            vk_copy_buffer(graphics_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size);

#endif