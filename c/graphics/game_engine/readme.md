

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


## TODO
* add mechanism to render both sides of an object (attribute in mesh layer entry?). add it for sponza curtains.
* add hot reloading to dynamically change mips ceiling/cap
* add texture/material de-duplication to reduce file size
* more complex sponza scene stress testing
  * add sponza ivy
  * add sponza trees
  * add hand placed lights?
* pak file stuff
  * pak file dynamic table of contents
  * find new name for pak file
  * pak file chain loading? or somehow referencing another pak file in a pak file.
  * pak file splitting in cooker
  * add a document describing the pak file format
* implement simple gi?
* add mips texture streaming (load lowest mip first, then ratchet up until vram is full)
