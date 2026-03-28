#include "vk_commands.h"
#include "core/logger.h"
#include "vk_devices.h"

VkCommandBuffer vk_begin_immediate_submit(graphics_t* r) {
    vkResetFences(r->core.device, 1, &r->transfer.fence);
    vkResetCommandBuffer(r->transfer.command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(r->transfer.command_buffer, &begin_info);
    return r->transfer.command_buffer;
}

void vk_end_immediate_submit(graphics_t* r) {
    vkEndCommandBuffer(r->transfer.command_buffer);

    VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &r->transfer.command_buffer,
    };

    vkQueueSubmit(r->core.graphics_queue, 1, &submit_info, r->transfer.fence);
    vkWaitForFences(r->core.device, 1, &r->transfer.fence, VK_TRUE, UINT64_MAX);
}

bool vk_create_commands(graphics_t* r) {
    queue_family_indices_t indices = find_queue_families(r->core.physical_device, r->core.surface);

    VkCommandPoolCreateInfo pool_info = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };

    if (vkCreateCommandPool(r->core.device, &pool_info, NULL, &r->command_pool) != VK_SUCCESS) {
        log_error("vulkan: failed to create command pool");
        return false;
    }

    VkCommandPoolCreateInfo imm_pool_info = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };
    vkCreateCommandPool(r->core.device, &imm_pool_info, NULL, &r->transfer.command_pool);

    VkCommandBufferAllocateInfo imm_alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = r->transfer.command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(r->core.device, &imm_alloc_info, &r->transfer.command_buffer);

    VkFenceCreateInfo imm_fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0,
    };
    vkCreateFence(r->core.device, &imm_fence_info, NULL, &r->transfer.fence);

    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = r->command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkAllocateCommandBuffers(r->core.device, &alloc_info, &r->frames[i].command_buffer) !=
            VK_SUCCESS) {
            log_error("vulkan: failed to allocate command buffers");
            return false;
        }
    }

    return true;
}

void vk_copy_buffer(graphics_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBuffer cmd         = vk_begin_immediate_submit(r);
    VkBufferCopy    copy_region = {.size = size};
    vkCmdCopyBuffer(cmd, src, dst, 1, &copy_region);
    vk_end_immediate_submit(r);
}

void vk_destroy_commands(graphics_t* r) {
    if (r->command_pool) {
        vkDestroyCommandPool(r->core.device, r->command_pool, NULL);
    }
}
