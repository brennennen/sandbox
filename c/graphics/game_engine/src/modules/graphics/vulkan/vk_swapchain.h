#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include "vk_types.h"

bool vk_create_swapchain(renderer_t* r, int width, int height);
void vk_recreate_swapchain(renderer_t* r, int width, int height);
void vk_destroy_swapchain(renderer_t* r);

#endif
