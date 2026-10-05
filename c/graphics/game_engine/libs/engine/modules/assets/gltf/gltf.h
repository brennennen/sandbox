#ifndef GLTF_H
#define GLTF_H

#include <stdint.h>
#include <string.h>


#include "libs/core/arena.h"
#include "shared/scene_types.h"

typedef struct {
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t mesh_count;
    uint32_t material_count;
    uint32_t texture_count;
} gltf_model_metrics_t;

typedef struct {
    const char** paths;
    uint32_t     count;
    uint32_t     capacity;
} texture_registry_t;

// Helper to check if a string is already in the registry
static bool texture_registry_add(arena_t* arena, texture_registry_t* reg, const char* new_path) {
    if (!new_path)
        return true; // Embedded textures (no path) are treated as unique

    for (uint32_t i = 0; i < reg->count; i++) {
        if (strcmp(reg->paths[i], new_path) == 0) {
            return false; // Already exists, don't count it
        }
    }

    // New texture! Add it.
    if (reg->count >= reg->capacity) {
        // Basic arena array push (assuming you initialize capacity to ~1024)
        const char** old_paths = reg->paths;
        reg->capacity *= 2;
        reg->paths = arena_push_array(arena, const char*, reg->capacity);
        memcpy(reg->paths, old_paths, reg->count * sizeof(const char*));
    }

    // Duplicate the string into the arena so it lives past the GLTF parsing
    size_t len        = strlen(new_path) + 1;
    char*  saved_path = arena_push_array(arena, char, len);
    memcpy(saved_path, new_path, len);

    reg->paths[reg->count++] = saved_path;
    return true; // Successfully added as new
}

bool gltf_measure_model(
    arena_t*              scratch,
    const char*           filepath,
    gltf_model_metrics_t* out_metrics,
    texture_registry_t*   texture_registry
);

bool gltf_bake_model(
    arena_t*      scratch_arena,
    const char*   filepath,
    scene_desc_t* out_scene,
    uint32_t      model_id,
    bool          use_fast_textures,
    bool          z_is_up
);

#endif // GLTF_H
