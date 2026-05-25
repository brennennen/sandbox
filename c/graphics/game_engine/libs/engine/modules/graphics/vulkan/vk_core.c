
#include "vk_types.h"
#include "volk.h"


void vk_set_debug_name(
    VkDevice     device,
    uint64_t     object_handle,
    VkObjectType object_type,
    const char*  name
) {
    if (vkSetDebugUtilsObjectNameEXT == NULL) {
        return;
    }

    VkDebugUtilsObjectNameInfoEXT name_info = {
        .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType   = object_type,
        .objectHandle = object_handle,
        .pObjectName  = name,
    };

    vkSetDebugUtilsObjectNameEXT(device, &name_info);
}