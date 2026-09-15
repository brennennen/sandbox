

#include "engine/modules/debug_imgui/debug_imgui.h"
#include "engine/modules/graphics/vulkan/vk_core.h"

#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
#include <cimgui_impl.h>
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif

#include "engine/core/logger.h"

// MARK: SDL3/Vulkan IMGUI

#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)

#include "engine/modules/graphics/vulkan/vk_commands.h"
#include "engine/modules/graphics/vulkan/vk_types.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

static VkDescriptorPool g_imgui_pool = VK_NULL_HANDLE;

static bool debug_imgui_sdl3_vulkan_init(game_engine_t* engine) {
    log_info("Initializing ImGui...");

    struct ImGuiContext* ctx = igCreateContext(NULL);
    ImGuiIO*             io  = igGetIO_Nil();

    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    igStyleColorsDark(NULL);

    SDL_Window* native_window = (SDL_Window*)platform_get_native_window(engine->platform);
    ImGui_ImplSDL3_InitForVulkan(native_window);

    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                    = 1000;
    pool_info.poolSizeCount              = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes                 = pool_sizes;

    vkCreateDescriptorPool(engine->graphics->core.device, &pool_info, NULL, &g_imgui_pool);

    ImGui_ImplVulkan_InitInfo init_info = {0};
    init_info.Instance                  = engine->graphics->core.instance;
    init_info.PhysicalDevice            = engine->graphics->core.physical_device;
    init_info.Device                    = engine->graphics->core.device;
    init_info.QueueFamily               = engine->graphics->core.graphics_queue_family;
    init_info.Queue                     = engine->graphics->core.graphics_queue;
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = g_imgui_pool;
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = 3;
    init_info.UseDynamicRendering       = true;

    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        &engine->graphics->display.format;
    init_info.PipelineInfoForViewports.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineInfoForViewports.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    init_info.PipelineInfoForViewports.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoForViewports.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        &engine->graphics->display.format;

    ImGui_ImplVulkan_Init(&init_info);

    return true;
}

bool debug_imgui_sdl3_vulkan_process_event(const void* native_event) {
    const SDL_Event* event = (const SDL_Event*)native_event;
    ImGui_ImplSDL3_ProcessEvent(event);
    ImGuiIO* io = igGetIO_Nil();
    if ((io->WantCaptureMouse &&
         (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_MOTION)) ||
        (io->WantCaptureKeyboard &&
         (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_TEXT_INPUT))) {
        return true;
    }

    return false;
}

void debug_imgui_sdl3_vulkan_begin_frame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    igNewFrame();
}

void debug_imgui_sdl3_vulkan_render(graphics_t* graphics) {
    igRender();
    VkCommandBuffer cmd = graphics->command_buffer;
    ImGui_ImplVulkan_RenderDrawData(igGetDrawData(), cmd, VK_NULL_HANDLE);
    ImGuiIO* io = igGetIO_Nil();
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        igUpdatePlatformWindows();
        igRenderPlatformWindowsDefault(NULL, NULL);
    }
}

void debug_imgui_sdl3_vulkan_shutdown(game_engine_t* engine) {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    igDestroyContext(NULL);
    // TODO: cleanup vk pools
    if (g_imgui_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(engine->graphics->core.device, g_imgui_pool, NULL);
        g_imgui_pool = VK_NULL_HANDLE;
    }
}

#endif

// MARK: debug_imgui api

bool debug_imgui_init(game_engine_t* engine) {
#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
    return debug_imgui_sdl3_vulkan_init(engine);
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif
}

bool debug_imgui_process_event(const void* native_event) {
#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
    return debug_imgui_sdl3_vulkan_process_event(native_event);
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif
}

void debug_imgui_begin_frame() {
#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
    return debug_imgui_sdl3_vulkan_begin_frame();
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif
}

void debug_imgui_render(graphics_t* graphics) {
#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
    debug_imgui_sdl3_vulkan_render(graphics);
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif
}

void debug_imgui_shutdown(game_engine_t* engine) {
#if defined(CIMGUI_USE_SDL3) && defined(CIMGUI_USE_VULKAN)
    debug_imgui_sdl3_vulkan_shutdown(engine);
#else
#error "Unhandled debug_ui imgui backend configuration!"
#endif
}
