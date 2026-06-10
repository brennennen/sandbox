#pragma once

#include "libs/core/arena.h"

#include "shared/pak_format.h"

bool process_ibl_textures(
    arena_t*           pak_arena,
    arena_t*           scratch_arena,
    const char*        hdri_path,
    environment_pak_t* out_pak_env
);
