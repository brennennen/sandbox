#include <stdlib.h>

#include "engine/core/logger.h"
#include "engine/modules/graphics/graphics.h"

#include "vk_commands.h"
#include "vk_gpu_allocator.h"
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
    present_mode_t abstract_present_mode,
    VkSwapchainKHR old_swapchain
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
        .oldSwapchain     = old_swapchain,
    };

    if (vkCreateSwapchainKHR(
            graphics->core.device, &create_info, NULL, &graphics->display.swapchain
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create swapchain");
        return false;
    }

    // vkDestroySwapchainKHR(graphics->core.device, old_swapchain, NULL);

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

VkCommandBuffer vk_begin_single_time_commands(graphics_t* r) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->core.device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &begin_info);
    return cmd;
}

void vk_end_single_time_commands(graphics_t* r, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &cmd,
    };

    vkQueueSubmit(r->core.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->core.graphics_queue);

    vkFreeCommandBuffers(r->core.device, r->command_pool, 1, &cmd);
}

static void vk_transition_depth_layout(graphics_t* r, VkImage image) {
    VkCommandBuffer cmd = vk_begin_immediate_submit(r);

    VkImageMemoryBarrier barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image            = image,
        .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
        .srcAccessMask    = 0,
        .dstAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    };

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
    );
    vk_end_immediate_submit(r);
}

bool vk_setup_depth_buffer(graphics_t* r, uint32_t width, uint32_t height) {
    VkImageCreateInfo depth_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .extent        = {width, height, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .format        = VK_FORMAT_D32_SFLOAT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
    };

    if (vkCreateImage(r->core.device, &depth_info, NULL, &r->display.depth_image) != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(r->core.device, r->display.depth_image, &mem_reqs);
    r->display.depth_alloc = gpu_heap_alloc(
        r->assets.display_heap, mem_reqs.size, mem_reqs.alignment
    );
    vkBindImageMemory(
        r->core.device,
        r->display.depth_image,
        r->assets.display_heap->memory,
        r->display.depth_alloc.offset
    );
    vk_transition_depth_layout(r, r->display.depth_image);

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = r->display.depth_image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = VK_FORMAT_D32_SFLOAT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1
        }
    };

    return vkCreateImageView(r->core.device, &view_info, NULL, &r->display.depth_view) ==
           VK_SUCCESS;
}

void vk_recreate_swapchain(graphics_t* graphics, int width, int height) {
    if (width == 0 || height == 0)
        return;
    vkDeviceWaitIdle(graphics->core.device);

    VkSwapchainKHR old_swapchain = graphics->display.swapchain;

    for (uint32_t i = 0; i < graphics->display.image_count; i++) {
        vkDestroyImageView(graphics->core.device, graphics->display.image_views[i], NULL);
    }
    vkDestroyImageView(graphics->core.device, graphics->display.depth_view, NULL);
    vkDestroyImage(graphics->core.device, graphics->display.depth_image, NULL);

    free(graphics->display.images);
    free(graphics->display.image_views);

    graphics->assets.display_heap->used = 0;

    vk_create_swapchain(
        graphics, width, height, graphics->display.abstract_present_mode, old_swapchain
    );
    vk_setup_depth_buffer(graphics, width, height);

    vkDestroySwapchainKHR(graphics->core.device, old_swapchain, NULL);
    log_info("vulkan: swapchain and depth buffer recreated for %dx%d", width, height);
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
