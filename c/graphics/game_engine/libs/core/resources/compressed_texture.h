#ifndef COMPRESSED_TEXTURE_H
#define COMPRESSED_TEXTURE_H

#include <stdint.h>

#include "core/arena.h"

typedef struct {
    uint8_t* compressed_bytes;
    uint32_t total_size;
    uint32_t mip_levels;
} compressed_texture_t;

// Compresses raw RGBA data into a full BC7 mip-chain.
// Allocates the final byte array onto the provided out_arena.
bool texture_compress_bc7(
    compressed_texture_t* compressed_texture,
    arena_t*              out_arena,
    const uint8_t*        raw_rgba_pixels,
    uint32_t              width,
    uint32_t              height,
    bool                  is_srgb
);

#endif
