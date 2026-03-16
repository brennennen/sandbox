

## Dependencies
* Requires VulkanSDK (interfaced through with volk)
* Requires glslc available from path
* SDL3




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


