#pragma once

#include "libs/core/arena.h"

bool cook_world(
    arena_t*    scratch_arena,
    const char* world_input_file,
    const char* world_output_file
);
