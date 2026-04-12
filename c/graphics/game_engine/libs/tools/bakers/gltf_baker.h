
#ifndef GLTF_BAKER_H
#define GLTF_BAKER_H

#include <stdbool.h>
#include <stdint.h>

#include "scene_types.h"

// A clean public API for the parser to use
bool bake_model(
    const char*   full_path,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    bool          opt_fast_textures,
    bool          opt_z_up
);

#endif // GLTF_BAKER_H
