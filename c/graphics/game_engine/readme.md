

## Dependencies
* Requires VulkanSDK (interfaced through with volk)
* Requires glslc available from path


## Folder Hierarchy
* common/: Header-only utilities, math libraries (linmath.h), and your custom types.h.
* core/: The heartbeat. Memory allocators, threading/task system, and logging.
* platform/: The "dirty" code. SDL3 abstractions, window handling, and file system I/O.
* renderer/: Your Vulkan implementation. Keep this strictly isolated from game logic.
* modules/: High-level systems like physics, audio, or ui.
* ext/ or .vendor/: Third-party code (SDL3, Vulkan headers).

## Modules
In C, a "module" should be defined by an Opaque Pointer (also known as the Pimpl idiom in C++).

The Rule: A module should never expose its internal structs in its public header.

    Bad: renderer.h shows the VkInstance handle.

    Good: renderer.h only shows typedef struct renderer_t renderer_t; and functions like renderer_draw_frame(renderer_t* r);.

This keeps your main.c from needing to include Vulkan headers, which significantly speeds up partial re-compilation.


## Context
// engine.h
typedef struct {
    platform_t* platform;
    renderer_t* renderer;
    task_system_t* tasks;
    // ...
} engine_context_t;

// main.c
int main() {
    engine_context_t ctx = {0};
    core_init(&ctx);
    renderer_init(ctx.renderer, ctx.platform->window);
    // ...
}


