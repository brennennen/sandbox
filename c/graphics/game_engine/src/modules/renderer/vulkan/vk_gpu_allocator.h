#ifndef VK_GPU_ALLOCATOR_H
#define VK_GPU_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#include "volk.h"  // Standard include, build system finds it in .vendor/volk

#include "renderer.h"

typedef struct gpu_heap_t gpu_heap_t;

typedef struct {
    uint64_t size;
    uint64_t offset;
    void* mapped_ptr;
} gpu_allocation_t;

gpu_heap_t* gpu_heap_create(renderer_t* r, VkDeviceSize size, VkMemoryPropertyFlags properties);

gpu_allocation_t gpu_heap_alloc(gpu_heap_t* heap, VkDeviceSize size, VkDeviceSize alignment);

#endif  // VK_GPU_ALLOCATOR_H
