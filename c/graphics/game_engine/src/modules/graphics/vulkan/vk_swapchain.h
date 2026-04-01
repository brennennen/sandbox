#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include "modules/graphics/graphics.h"
#include "vk_types.h"

VkPresentModeKHR choose_swapchain_present_mode(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR     surface,
    present_mode_t   requested_mode
);

bool vk_setup_depth_buffer(graphics_t* r, uint32_t width, uint32_t height);

bool vk_create_swapchain(
    graphics_t*    graphics,
    int            width,
    int            height,
    present_mode_t present_mode,
    VkSwapchainKHR old_swapchain
);

void vk_recreate_swapchain(graphics_t* graphics, int width, int height);
void vk_destroy_swapchain(graphics_t* graphics);

#endif
