#ifndef VK_GPU_ALLOCATOR_H
#define VK_GPU_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#include "volk.h"

// #include "modules/graphics/graphics.h"

#include "vk_types.h"

// typedef struct gpu_heap_t gpu_heap_t;

gpu_heap_t* gpu_heap_create(graphics_t* r, VkDeviceSize size, VkMemoryPropertyFlags properties);

gpu_allocation_t gpu_heap_alloc(gpu_heap_t* heap, VkDeviceSize size, VkDeviceSize alignment);

#endif // VK_GPU_ALLOCATOR_H
