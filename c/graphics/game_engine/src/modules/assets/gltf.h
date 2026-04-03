#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

#include <stdbool.h>

#include "core/scene.h"
#include "modules/graphics/graphics.h"
#include "modules/graphics/graphics_types.h"


bool load_gltf_scene(
    const char*      filepath,
    scene_t*         out_scene,
    graphics_t*      gfx,
    texture_handle_t default_tex
);

#endif
