#include "vk_swapchain.h"
#include <stdlib.h>
#include "core/logger.h"

bool vk_create_swapchain(renderer_t* r, int width, int height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->physical_device, r->surface, &capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(r->physical_device, r->surface, &format_count, NULL);
    VkSurfaceFormatKHR formats[format_count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(r->physical_device, r->surface, &format_count, formats);

    // Default to first, but prefer SRGB for better color accuracy
    VkSurfaceFormatKHR selected_format = formats[0];
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_format = formats[i];
            break;
        }
    }

    r->swapchain_format = selected_format.format;
    r->swapchain_extent.width = (uint32_t)width;
    r->swapchain_extent.height = (uint32_t)height;

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = r->surface,
        .minImageCount = 3,  // Triple buffering
        .imageFormat = selected_format.format,
        .imageColorSpace = selected_format.colorSpace,
        .imageExtent = {(uint32_t)width, (uint32_t)height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,  // V-Sync
        .clipped = VK_TRUE,
    };

    if (vkCreateSwapchainKHR(r->device, &create_info, NULL, &r->swapchain) != VK_SUCCESS) {
        log_error("vulkan: failed to create swapchain");
        return false;
    }

    // Fetch the images created by the swapchain
    vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchain_image_count, NULL);
    r->swapchain_images = malloc(sizeof(VkImage) * r->swapchain_image_count);
    vkGetSwapchainImagesKHR(
        r->device, r->swapchain, &r->swapchain_image_count, r->swapchain_images
    );

    // Create a View for every image
    r->swapchain_image_views = malloc(sizeof(VkImageView) * r->swapchain_image_count);
    for (uint32_t i = 0; i < r->swapchain_image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = r->swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = selected_format.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1
            },
        };
        vkCreateImageView(r->device, &view_info, NULL, &r->swapchain_image_views[i]);
    }

    log_info("vulkan: swapchain created with %u images", r->swapchain_image_count);
    return true;
}

void vk_recreate_swapchain(renderer_t* r, int width, int height) {
    if (width == 0 || height == 0) {
        return;
    }
    vkDeviceWaitIdle(r->device);            // Wait for the GPU to finish using the old swapchain
    vk_destroy_swapchain(r);                // Clean up the old swapchain
    vk_create_swapchain(r, width, height);  // Create the new one with the updated dimensions
    log_info("vulkan: swapchain recreated for %dx%d", width, height);
}

void vk_destroy_swapchain(renderer_t* r) {
    if (!r->device || !r->swapchain) return;

    for (uint32_t i = 0; i < r->swapchain_image_count; i++) {
        vkDestroyImageView(r->device, r->swapchain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(r->device, r->swapchain, NULL);

    free(r->swapchain_images);
    free(r->swapchain_image_views);

    r->swapchain_images = NULL;
    r->swapchain_image_views = NULL;
}
