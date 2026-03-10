
#ifndef VK_DEVICES_H
#define VK_DEVICES_H

#include "vk_types.h"
#include "volk.h"
#include <stdint.h>


typedef struct {
    uint32_t graphics_family;
    bool     has_graphics;
} queue_family_indices_t;

bool vk_create_instance(renderer_t* r);
bool vk_pick_physical_device(renderer_t* r);
bool vk_create_logical_device(renderer_t* r);

queue_family_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

#endif // VK_DEVICES_H
