

## Dependencies
* Requires VulkanSDK (interfaced through with volk)
* Requires glslc available from path
* SDL3


zig build
zig build cook -- ./.assets/test_zone.world ./.assets/test_zone.pak
zig build cook -- ./.assets/sponza.world ./.assets/sponza.pak
zig build run
