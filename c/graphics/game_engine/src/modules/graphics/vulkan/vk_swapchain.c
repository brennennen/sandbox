#include <stdlib.h>

#include "core/logger.h"
#include "modules/graphics/graphics.h"
#include "vk_swapchain.h"

VkPresentModeKHR choose_swapchain_present_mode(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR     surface,
    present_mode_t   requested_mode
) {
    VkPresentModeKHR desired_vk_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (requested_mode == PRESENT_MODE_IMMEDIATE) {
        desired_vk_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    } else if (requested_mode == PRESENT_MODE_MAILBOX) {
        desired_vk_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

    if (present_mode_count != 0) {
        VkPresentModeKHR present_modes[present_mode_count];
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, &present_mode_count, present_modes
        );
        for (uint32_t i = 0; i < present_mode_count; i++) {
            if (present_modes[i] == desired_vk_mode) {
                return desired_vk_mode;
            }
        }
    }

    if (desired_vk_mode != VK_PRESENT_MODE_FIFO_KHR) {
        log_warn("Requested present mode not supported by hardware, falling back to V-Sync");
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool vk_create_swapchain(
    graphics_t*    graphics,
    int            width,
    int            height,
    present_mode_t abstract_present_mode
) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        graphics->core.physical_device, graphics->core.surface, &capabilities
    );

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        graphics->core.physical_device, graphics->core.surface, &format_count, NULL
    );
    VkSurfaceFormatKHR formats[format_count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        graphics->core.physical_device, graphics->core.surface, &format_count, formats
    );

    // Default to first, but prefer SRGB for better color accuracy
    VkSurfaceFormatKHR selected_format = formats[0];
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_format = formats[i];
            break;
        }
    }

    graphics->display.format        = selected_format.format;
    graphics->display.extent.width  = (uint32_t)width;
    graphics->display.extent.height = (uint32_t)height;

    VkPresentModeKHR present_mode = choose_swapchain_present_mode(
        graphics->core.physical_device, graphics->core.surface, abstract_present_mode
    );
    graphics->display.present_mode = present_mode;

    VkSwapchainCreateInfoKHR create_info = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = graphics->core.surface,
        .minImageCount    = 3, // Triple buffering
        .imageFormat      = selected_format.format,
        .imageColorSpace  = selected_format.colorSpace,
        .imageExtent      = {(uint32_t)width, (uint32_t)height},
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = present_mode,
        .clipped          = VK_TRUE,
    };

    if (vkCreateSwapchainKHR(
            graphics->core.device, &create_info, NULL, &graphics->display.swapchain
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create swapchain");
        return false;
    }

    // Fetch the images created by the swapchain
    vkGetSwapchainImagesKHR(
        graphics->core.device, graphics->display.swapchain, &graphics->display.image_count, NULL
    );
    graphics->display.images = malloc(sizeof(VkImage) * graphics->display.image_count);
    vkGetSwapchainImagesKHR(
        graphics->core.device,
        graphics->display.swapchain,
        &graphics->display.image_count,
        graphics->display.images
    );

    // Create a View for every image
    graphics->display.image_views = malloc(sizeof(VkImageView) * graphics->display.image_count);
    for (uint32_t i = 0; i < graphics->display.image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = graphics->display.images[i],
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = selected_format.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1
            },
        };
        vkCreateImageView(
            graphics->core.device, &view_info, NULL, &graphics->display.image_views[i]
        );
    }

    log_info("vulkan: swapchain created with %u images", graphics->display.image_count);
    return true;
}

void vk_recreate_swapchain(
    graphics_t*    graphics,
    int            width,
    int            height,
    present_mode_t present_mode
) {
    if (width == 0 || height == 0) {
        return;
    }
    vkDeviceWaitIdle(graphics->core.device); // Wait for the GPU to finish using the old swapchain
    vk_destroy_swapchain(graphics);
    vk_create_swapchain(graphics, width, height, present_mode);
    log_info("vulkan: swapchain recreated for %dx%d", width, height);
}

void vk_destroy_swapchain(graphics_t* graphics) {
    if (!graphics->core.device || !graphics->display.swapchain)
        return;

    for (uint32_t i = 0; i < graphics->display.image_count; i++) {
        vkDestroyImageView(graphics->core.device, graphics->display.image_views[i], NULL);
    }

    vkDestroySwapchainKHR(graphics->core.device, graphics->display.swapchain, NULL);

    free(graphics->display.images);
    free(graphics->display.image_views);

    graphics->display.images      = NULL;
    graphics->display.image_views = NULL;
}
