#ifndef PAK_LOADER_H
#define PAK_LOADER_H

#include <stdbool.h>
#include <stdint.h>

#include "engine/core/game_engine.h"
#include "engine/modules/graphics/graphics_types.h"

typedef struct graphics_t    graphics_t;
typedef struct scene_s       scene_t;
typedef struct environment_s environment_t;

bool pak_loader_load_world(
    graphics_t*    graphics,
    scene_t*       out_scene,
    environment_t* out_env,
    const char*    pak_path
);

bool             pak_loader_load_scene(graphics_t* graphics, scene_t* scene, const char* pak_path);
texture_handle_t pak_loader_load_skybox(graphics_t* graphics, const char* pak_path);

#endif
