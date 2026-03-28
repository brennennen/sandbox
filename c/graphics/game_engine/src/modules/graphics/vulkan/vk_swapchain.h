#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include "vk_types.h"

bool vk_create_swapchain(graphics_t* graphics, int width, int height);
void vk_recreate_swapchain(graphics_t* graphics, int width, int height);
void vk_destroy_swapchain(graphics_t* graphics);

#endif
