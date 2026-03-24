#include "vk_commands.h"
#include "core/logger.h"
#include "vk_devices.h"

bool vk_create_commands(graphics_t* r) {
    queue_family_indices_t indices = find_queue_families(r->physical_device, r->surface);

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };

    if (vkCreateCommandPool(r->device, &pool_info, NULL, &r->command_pool) != VK_SUCCESS) {
        log_error("vulkan: failed to create command pool");
        return false;
    }

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = r->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(r->device, &alloc_info, &r->command_buffer) != VK_SUCCESS) {
        log_error("vulkan: failed to allocate main command buffer");
        return false;
    }

    return true;
}

void vk_copy_buffer(graphics_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer temp_cmd;
    vkAllocateCommandBuffers(r->device, &alloc_info, &temp_cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(temp_cmd, &begin_info);
    VkBufferCopy copy_region = {.size = size};
    vkCmdCopyBuffer(temp_cmd, src, dst, 1, &copy_region);
    vkEndCommandBuffer(temp_cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &temp_cmd,
    };

    vkQueueSubmit(r->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->graphics_queue);

    vkFreeCommandBuffers(r->device, r->command_pool, 1, &temp_cmd);
}

void vk_destroy_commands(graphics_t* r) {
    if (r->command_pool) {
        vkDestroyCommandPool(r->device, r->command_pool, NULL);
    }
}
