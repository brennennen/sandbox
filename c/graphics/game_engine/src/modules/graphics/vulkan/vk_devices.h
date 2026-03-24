
#ifndef VK_DEVICES_H
#define VK_DEVICES_H

#include <stdint.h>

#include "vk_types.h"
#include "volk.h"

#include "platform/platform.h"

typedef struct {
    uint32_t graphics_family;
    bool     has_graphics;
} queue_family_indices_t;

bool vk_create_instance(graphics_t* r, platform_t* platform);
bool vk_pick_physical_device(graphics_t* r);
bool vk_create_logical_device(graphics_t* r);

queue_family_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

#endif // VK_DEVICES_H
