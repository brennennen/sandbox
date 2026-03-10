#ifndef VK_COMMANDS_H
#define VK_COMMANDS_H

#include "vk_types.h"

bool vk_create_commands(renderer_t* r);
void vk_destroy_commands(renderer_t* r);
void vk_copy_buffer(renderer_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size);

#endif