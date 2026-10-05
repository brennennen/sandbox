#ifndef COOKER_H
#define COOKER_H

#include <string.h>

#include "libs/core/arena.h"

bool cook_world(
    arena_t*    scratch_arena,
    const char* world_input_file,
    const char* world_output_file
);



#endif // COOKER_H
