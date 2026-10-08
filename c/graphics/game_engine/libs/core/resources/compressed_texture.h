#ifndef COMPRESSED_TEXTURE_H
#define COMPRESSED_TEXTURE_H

#include <stdint.h>

#include "core/arena.h"

typedef struct {
    uint8_t* compressed_bytes;
    uint32_t total_size;
    uint32_t mip_levels;
} compressed_texture_t;

uint32_t calculate_mip_count(uint32_t w, uint32_t h);

bool texture_compress_bc7(
    compressed_texture_t* compressed_texture,
    const uint8_t*        raw_rgba_pixels,
    uint32_t              width,
    uint32_t              height,
    bool                  is_srgb
);

uint8_t* bc7_compress_and_cache(
    const char* cache_path,
    uint8_t*    raw_pixels,
    int         w,
    int         h,
    uint32_t*   out_size,
    uint32_t*   out_mip_levels
);

#endif
