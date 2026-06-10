/**
 *
 * https://docs.vulkan.org/spec/latest/chapters/pipelines.html
 */

#include <stdio.h>
#include <stdlib.h>

#include "engine/core/logger.h"
#include "engine/modules/graphics/graphics_types.h"

#include "vk_core.h"
#include "vk_pipeline.h"

VkShaderModule vk_create_shader_module(VkDevice device, const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        log_error("vulkan: failed to open shader file: %s", path);
        return VK_NULL_HANDLE;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    uint32_t* code = malloc(size);
    fread(code, 1, size, file);
    fclose(file);

    VkShaderModuleCreateInfo create_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode    = code,
    };

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, NULL, &shader_module) != VK_SUCCESS) {
        log_error("vulkan: failed to create shader module from: %s", path);
        free(code);
        return VK_NULL_HANDLE;
    }

    free(code);
    return shader_module;
}

static VkPipeline create_skybox_pipeline(graphics_t* graphics, VkFormat color_format) {
    VkShaderModule vert_mod = vk_create_shader_module(
        graphics->core.device, "shaders/core/skybox.vert.spv"
    );
    VkShaderModule frag_mod = vk_create_shader_module(
        graphics->core.device, "shaders/core/skybox.frag.spv"
    );

    VkPipelineShaderStageCreateInfo stages[2] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_mod,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_mod,
            .pName  = "main",
        }
    };

    // bufferless rendering
    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions    = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions    = NULL
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // 36 vertices = 12 triangles
        .primitiveRestartEnable = VK_FALSE
    };

    // no culling, we are inside the cube
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth   = 1.0f,
        .cullMode    = VK_CULL_MODE_NONE,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
    };

    // don't write to depth, and use LESS_OR_EQUAL
    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL,
    };

    VkPipelineRenderingCreateInfo rendering_info = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &color_format, //&graphics->display.format,
        .depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT,
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering_info,
        .stageCount          = 2,
        .pStages             = stages,
        .pVertexInputState   = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState =
            &(VkPipelineViewportStateCreateInfo){
                .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount  = 1,
            },
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_info,
        .layout              = graphics->pipelines.layout,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(
            graphics->core.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create skybox pipeline");
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(graphics->core.device, vert_mod, NULL);
    vkDestroyShaderModule(graphics->core.device, frag_mod, NULL);
    return pipeline;
}

static VkPipeline create_pipeline_internal(
    graphics_t*         graphics,
    VkPrimitiveTopology topology,
    VkPolygonMode       polygon_mode,
    VkCullModeFlags     cull_mode,
    bool                depth_test,
    VkPipelineLayout    layout,
    const char*         vert_path,
    const char*         frag_path,
    VkFormat            color_format,
    const char*         debug_name
) {

    VkShaderModule vert_mod = vk_create_shader_module(graphics->core.device, vert_path);
    VkShaderModule frag_mod = vk_create_shader_module(graphics->core.device, frag_path);

    VkPipelineShaderStageCreateInfo stages[2] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_mod,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_mod,
            .pName  = "main",
        }
    };

    VkVertexInputBindingDescription binding_desc = {
        .binding = 0, .stride = sizeof(vertex_t), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attribute_descriptions[] = {
        // Location, Binding, Format, Offset
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, pos)},
        {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, uv)},
        {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, normal)},
        {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, tangent)}
    };

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &binding_desc,
        .vertexAttributeDescriptionCount = sizeof(attribute_descriptions) /
                                           sizeof(attribute_descriptions[0]),
        .pVertexAttributeDescriptions = attribute_descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = topology,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = polygon_mode,
        .lineWidth   = 1.0f,
        .cullMode    = cull_mode,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = depth_test ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = depth_test ? VK_TRUE : VK_FALSE,
        .depthCompareOp   = VK_COMPARE_OP_LESS,
    };

    VkPipelineRenderingCreateInfo rendering_info = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &color_format,
        .depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT,
    };

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering_info,
        .stageCount          = 2,
        .pStages             = stages,
        .pVertexInputState   = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState =
            &(VkPipelineViewportStateCreateInfo){
                .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount  = 1,
            },
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_info,
        .layout              = layout, // graphics->pipelines.layout
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(
            graphics->core.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create pipeline with topology %d", topology);
        return VK_NULL_HANDLE;
    }

    vk_set_debug_name(
        graphics->core.device, (uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, debug_name
    );

    vkDestroyShaderModule(graphics->core.device, vert_mod, NULL);
    vkDestroyShaderModule(graphics->core.device, frag_mod, NULL);
    return pipeline;
}

/**
 * Creates the Descriptor Set Layout for global, per-frame data (Set 0).
 * This layout dictates how environment and camera data is bound to the GPU.
 * It is expected to be bound once at the start of the frame.
 *
 * Bindings:
 * - 0: Uniform Buffer (Camera matrices, time, etc.)
 * - 1: Combined Image Sampler (HDRI Skybox)
 * - 2: Combined Image Sampler (IBL Irradiance Map)
 * - 3: Combined Image Sampler (IBL Prefiltered Specular Map)
 */
static bool init_global_descriptor_layout(graphics_t* graphics) {
    VkDescriptorSetLayoutBinding global_ubo_binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding skybox_binding = {
        .binding         = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding irradiance_binding = {
        .binding         = 2,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding prefilter_binding = {
        .binding         = 3,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding global_bindings[] = {
        global_ubo_binding,
        skybox_binding,
        irradiance_binding,
        prefilter_binding,
    };

    VkDescriptorSetLayoutCreateInfo global_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
        .pBindings    = global_bindings,
    };
    if (vkCreateDescriptorSetLayout(
            graphics->core.device, &global_info, NULL, &graphics->pipelines.global_set_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create global descriptor set layout");
        return false;
    }
    return true;
}

/**
 * Creates the Descriptor Set Layout for per-object material data (Set 1).
 * This layout defines the texture inputs required for the PBR shader.
 * It is expected to be bound before each draw call if the material changes.
 *
 * Bindings:
 * - 0: Combined Image Sampler (Albedo/Base Color)
 * - 1: Combined Image Sampler (Normal Map)
 * - 2: Combined Image Sampler (Packed AO/Roughness/Metallic)
 */
static bool init_object_descriptor_layout(graphics_t* graphics) {
    VkDescriptorSetLayoutBinding albedo_binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding normal_binding = {
        .binding         = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding ao_metallic_roughness_binding = {
        .binding         = 2,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding bindings[] = {
        albedo_binding,
        normal_binding,
        ao_metallic_roughness_binding,
    };

    VkDescriptorSetLayoutCreateInfo info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings    = bindings,
    };

    if (vkCreateDescriptorSetLayout(
            graphics->core.device, &info, NULL, &graphics->pipelines.object_set_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create object descriptor set layout");
        return false;
    }
    return true;
}

/**
 * Constructs the Pipeline Layout for the main forward/PBR rendering pass.
 * Merges the Global (Set 0) and Object (Set 1) descriptor layouts, and defines
 * the push constant range used for per-object transforms and materials.
 */
static bool init_main_pipeline_layout(graphics_t* graphics) {
    VkDescriptorSetLayout layouts[] = {
        graphics->pipelines.global_set_layout,
        graphics->pipelines.object_set_layout,
    };

    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(push_constants_t),
    };

    VkPipelineLayoutCreateInfo info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 2,
        .pSetLayouts            = layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_constant,
    };

    if (vkCreatePipelineLayout(graphics->core.device, &info, NULL, &graphics->pipelines.layout) !=
        VK_SUCCESS) {
        log_error("vulkan: failed to create main pipeline layout");
        return false;
    }
    return true;
}

/**
 * Initializes both the descriptor set and pipeline layouts for post-processing.
 * Sets up a minimalist pipeline meant for fullscreen quad drawing.
 *
 * Bindings (Set 0):
 * - 0: Combined Image Sampler (The resolved main scene HDR render target)
 */
static bool init_post_process_layouts(graphics_t* graphics) {
    VkDescriptorSetLayoutBinding binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutCreateInfo set_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings    = &binding,
    };

    if (vkCreateDescriptorSetLayout(
            graphics->core.device, &set_info, NULL, &graphics->pipelines.post_process_set_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create post process descriptor set layout");
        return false;
    }

    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &graphics->pipelines.post_process_set_layout,
        .pushConstantRangeCount = 0,
    };

    if (vkCreatePipelineLayout(
            graphics->core.device, &pipeline_info, NULL, &graphics->pipelines.post_process_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create post process pipeline layout");
        return false;
    }
    return true;
}

/**
 * Master initialization routine for all Vulkan pipeline and descriptor layouts.
 * Orchestrates the creation of the memory contracts between the CPU and the shaders.
 * Must be called before graphics pipelines are compiled.
 */
static bool init_pipeline_layouts(graphics_t* graphics) {
    if (!init_global_descriptor_layout(graphics)) {
        return false;
    }
    if (!init_object_descriptor_layout(graphics)) {
        return false;
    }
    if (!init_main_pipeline_layout(graphics)) {
        return false;
    }
    if (!init_post_process_layouts(graphics)) {
        return false;
    }
    return true;
}

bool vk_create_graphics_pipeline(graphics_t* graphics) {
    if (!init_pipeline_layouts(graphics)) {
        return false;
    }

    graphics->pipelines.forward_lit = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        true,
        graphics->pipelines.layout,
        "shaders/core/mesh.vert.spv",
        "shaders/core/pbr.frag.spv",
        VK_FORMAT_R16G16B16A16_SFLOAT,
        "graphics->pipelines.forward_lit"
    );
    graphics->pipelines.skybox = create_skybox_pipeline(graphics, VK_FORMAT_R16G16B16A16_SFLOAT);
    vk_set_debug_name(
        graphics->core.device,
        (uint64_t)graphics->pipelines.skybox,
        VK_OBJECT_TYPE_PIPELINE,
        "graphics->pipelines.skybox"
    );
    graphics->pipelines.post_process = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        false,
        graphics->pipelines.post_process_layout,
        "shaders/post_process/fullscreen.vert.spv",
        "shaders/post_process/post_process.frag.spv",
        graphics->display.format,
        "graphics->pipelines.post_process"
    );
    graphics->pipelines.line = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        true,
        graphics->pipelines.layout,
        "shaders/core/line.vert.spv",
        "shaders/core/line.frag.spv",
        VK_FORMAT_R16G16B16A16_SFLOAT,
        "graphics->pipelines.line"
    );
    graphics->pipelines.debug_forward_lit = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        true,
        graphics->pipelines.layout,
        "shaders/core/mesh.vert.spv",
        "shaders/core/debug_pbr.frag.spv",
        VK_FORMAT_R16G16B16A16_SFLOAT,
        "graphics->pipelines.debug_forward_lit"
    );
    graphics->pipelines.debug_wireframe = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_LINE,
        VK_CULL_MODE_NONE,
        true,
        graphics->pipelines.layout,
        "shaders/core/mesh.vert.spv",
        "shaders/core/debug_wireframe.frag.spv",
        VK_FORMAT_R16G16B16A16_SFLOAT,
        "graphics->pipelines.debug_wireframe"
    );
    graphics->pipelines.debug_sdr = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        true,
        graphics->pipelines.layout,
        "shaders/core/mesh.vert.spv",
        "shaders/core/pbr.frag.spv",
        graphics->display.format,
        "graphics->pipelines.debug_sdr"
    );

    return (
        graphics->pipelines.forward_lit != VK_NULL_HANDLE &&
        graphics->pipelines.skybox != VK_NULL_HANDLE &&
        graphics->pipelines.post_process != VK_NULL_HANDLE &&
        graphics->pipelines.debug_wireframe != VK_NULL_HANDLE &&
        graphics->pipelines.debug_forward_lit != VK_NULL_HANDLE &&
        graphics->pipelines.line != VK_NULL_HANDLE
    );
}

void vk_destroy_graphics_pipeline(graphics_t* graphics) {
    if (graphics->pipelines.forward_lit) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.forward_lit, NULL);
    }
    if (graphics->pipelines.transparent) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.transparent, NULL);
    }
    if (graphics->pipelines.skybox) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.skybox, NULL);
    }
    if (graphics->pipelines.post_process) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.post_process, NULL);
    }
    if (graphics->pipelines.line) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.line, NULL);
    }
    if (graphics->pipelines.debug_forward_lit) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.debug_forward_lit, NULL);
    }
    if (graphics->pipelines.debug_wireframe) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.debug_wireframe, NULL);
    }
    if (graphics->pipelines.debug_sdr) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.debug_sdr, NULL);
    }

    if (graphics->pipelines.layout) {
        vkDestroyPipelineLayout(graphics->core.device, graphics->pipelines.layout, NULL);
    }
    if (graphics->pipelines.post_process_layout) {
        vkDestroyPipelineLayout(
            graphics->core.device, graphics->pipelines.post_process_layout, NULL
        );
    }
    if (graphics->pipelines.global_set_layout) {
        vkDestroyDescriptorSetLayout(
            graphics->core.device, graphics->pipelines.global_set_layout, NULL
        );
    }
    if (graphics->pipelines.object_set_layout) {
        vkDestroyDescriptorSetLayout(
            graphics->core.device, graphics->pipelines.object_set_layout, NULL
        );
    }
    if (graphics->pipelines.post_process_set_layout) {
        vkDestroyDescriptorSetLayout(
            graphics->core.device, graphics->pipelines.post_process_set_layout, NULL
        );
    }
}
