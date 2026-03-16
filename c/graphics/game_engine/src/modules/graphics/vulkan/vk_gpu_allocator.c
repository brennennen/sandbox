
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "core/logger.h"
#include "modules/graphics/graphics.h"
#include "volk.h"

#include "vk_gpu_allocator.h"
#include "vk_types.h"

static inline VkDeviceSize align_up(VkDeviceSize addr, VkDeviceSize align) {
    return (addr + align - 1) & ~(align - 1);
}

gpu_heap_t* gpu_heap_create(renderer_t* r, VkDeviceSize size, VkMemoryPropertyFlags properties) {
    if (!r) {
        printf("[ERROR]: Renderer is NULL\n");
        return NULL;
    }
    if (!r->device) {
        printf("[ERROR]: Device is NULL in gpu_heap_create\n");
        return NULL;
    }

    gpu_heap_t* heap = malloc(sizeof(gpu_heap_t));
    if (!heap)
        return NULL;

    printf("[DEBUG]: Creating GPU Heap of size %llu\n", size);
    heap->capacity    = size;
    heap->offset      = 0;
    heap->mapped_data = nullptr;

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(r->physical_device, &mem_props);

    uint32_t type_filter = 0xFFFFFFFF;
    uint32_t type_index  = 0;
    bool     found       = false;
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if (mem_props.memoryTypes[i].propertyFlags & properties) {
            type_index = i;
            found      = true;
            break;
        }
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize  = size,
        .memoryTypeIndex = type_index
    };

    if (vkAllocateMemory(r->device, &alloc_info, nullptr, &heap->memory) != VK_SUCCESS) {
        free(heap);
        return nullptr;
    }

    if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vkMapMemory(r->device, heap->memory, 0, size, 0, &heap->mapped_data);
    } else {
        heap->mapped_data = NULL;
    }

    return heap;
}

gpu_allocation_t gpu_heap_alloc(gpu_heap_t* heap, uint64_t size, uint64_t alignment) {
    uint64_t aligned_offset = (heap->offset + alignment - 1) & ~(alignment - 1);

    if (aligned_offset + size > heap->capacity) {
        printf(
            "[ERROR]: GPU Heap Overflow! Requested %llu, but only %llu left.\n",
            size,
            heap->capacity - aligned_offset
        );
        return (gpu_allocation_t){0};
    }

    gpu_allocation_t alloc = {
        .size       = size,
        .offset     = aligned_offset,
        .mapped_ptr = heap->mapped_data ? (char*)heap->mapped_data + aligned_offset : NULL
    };

    heap->offset = aligned_offset + size;
    heap->used += size;
    float percent = ((float)heap->offset / (float)heap->capacity) * 100.0f;
    printf(
        "[DEBUG]: GPU Alloc: %llu bytes at offset %llu (%.2f%% of heap used)\n",
        size,
        aligned_offset,
        percent
    );

    return alloc;
}
