#pragma once

#include "vk_types.h"
#include "volk.h"

void vk_set_debug_name(
    VkDevice     device,
    uint64_t     object_handle,
    VkObjectType object_type,
    const char*  name
);
