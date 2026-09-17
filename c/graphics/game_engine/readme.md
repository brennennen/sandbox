

## Dependencies
* VulkanSDK - gpu api
  * volk - wrapper to make vulkan easier to use
* glslc - shader compiler
  * requires manually adding to path
* SDL3 - platform (window/input/sound) abstraction layer
* cgtlf - c gtlf parsing library
* cimgui - c wrapper around imgui (c++)
  * dear imgui (docking branch) - easy to integrate immediate mode ui lib
* bc7enc - block compression gpu friendly image compression demo library


zig build
zig build cook -- ./.assets/test_zone.world ./.assets/test_zone.pak
zig build cook -- ./.assets/sponza.world ./.assets/sponza.pak
zig build run


