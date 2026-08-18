

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bc7/bc7enc.h"
#include "stb_image_resize2.h"

#include "libs/core/arena.h"
#include "libs/engine/core/logger.h"

#include "core/resources/compressed_texture.h"

bool texture_compress_bc7_2(
    compressed_texture_t* compressed_texture,
    arena_t*              out_arena,
    const uint8_t*        raw_rgba_pixels,
    uint32_t              width,
    uint32_t              height,
    bool                  is_srgb
) {
    return false;
}

static inline void bc7enc_compress_block_params_init_linear_weights_gltf(
    bc7enc_compress_block_params* p
) {
    p->m_perceptual = 0;
    p->m_weights[0] = 1;
    p->m_weights[1] = 1;
    p->m_weights[2] = 1;
    p->m_weights[3] = 1;
}

static inline void bc7enc_compress_block_params_init_perceptual_weights_gltf(
    bc7enc_compress_block_params* p
) {
    p->m_perceptual = 1;
    p->m_weights[0] = 128;
    p->m_weights[1] = 64;
    p->m_weights[2] = 16;
    p->m_weights[3] = 32;
}

static inline void bc7enc_compress_block_params_init_gltf(bc7enc_compress_block_params* p) {
    p->m_max_partitions_mode                  = 0; // BC7ENC_MAX_PARTITIONS1
    p->m_try_least_squares                    = 1;
    p->m_mode_partition_estimation_filterbank = 1;
    p->m_uber_level                           = 0;
    p->m_use_mode5_for_alpha                  = 1;
    p->m_use_mode7_for_alpha                  = 1;
}

static uint32_t calculate_bc7_mip_chain_size(uint32_t w, uint32_t h, uint32_t mip_levels) {
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < mip_levels; i++) {
        uint32_t blocks_x = (w + 3) / 4;
        uint32_t blocks_y = (h + 3) / 4;
        total_size += blocks_x * blocks_y * 16; // 16 bytes per BC7 block
        w = (w > 1) ? w / 2 : 1;
        h = (h > 1) ? h / 2 : 1;
    }
    return total_size;
}

bool texture_compress_bc7(
    compressed_texture_t* out_texture,
    arena_t*              out_arena,
    const uint8_t*        raw_rgba_pixels,
    uint32_t              width,
    uint32_t              height,
    bool                  is_srgb
) {
    if (!out_texture || !out_arena || !raw_rgba_pixels || width == 0 || height == 0) {
        return false;
    }

    uint32_t max_dim    = (width > height) ? width : height;
    uint32_t mip_levels = (uint32_t)(floorf(log2f((float)max_dim))) + 1;
    uint32_t total_size = calculate_bc7_mip_chain_size(width, height, mip_levels);

    uint8_t* bc7_buffer = malloc(total_size);
    if (!bc7_buffer) {
        log_error("CRITICAL: OS refused to malloc %u bytes for BC7 compression!", total_size);
        return false;
    }

    bc7enc_compress_block_params pack_params;
    bc7enc_compress_block_params_init_gltf(&pack_params);

    if (is_srgb) {
        bc7enc_compress_block_params_init_perceptual_weights_gltf(&pack_params);
    } else {
        bc7enc_compress_block_params_init_linear_weights_gltf(&pack_params);
    }

    pack_params.m_max_partitions_mode = 0;
    pack_params.m_try_least_squares   = 0;
    pack_params.m_use_mode5_for_alpha = 1;
    pack_params.m_use_mode7_for_alpha = 1;

    uint8_t* current_mip_pixels = malloc(width * height * 4);
    if (!current_mip_pixels) {
        return false;
    }
    memcpy(current_mip_pixels, raw_rgba_pixels, width * height * 4);

    uint32_t dst_offset = 0;
    uint32_t mip_w      = width;
    uint32_t mip_h      = height;

    for (uint32_t mip = 0; mip < mip_levels; mip++) {
        uint32_t blocks_x = (mip_w + 3) / 4;
        uint32_t blocks_y = (mip_h + 3) / 4;

        for (uint32_t by = 0; by < blocks_y; by++) {
            for (uint32_t bx = 0; bx < blocks_x; bx++) {
                uint32_t block_pixels[16] = {0};

                for (uint32_t py = 0; py < 4; py++) {
                    for (uint32_t px = 0; px < 4; px++) {
                        uint32_t global_x = bx * 4 + px;
                        uint32_t global_y = by * 4 + py;

                        global_x = (global_x < mip_w) ? global_x : mip_w - 1;
                        global_y = (global_y < mip_h) ? global_y : mip_h - 1;

                        uint32_t src_idx = (global_y * mip_w + global_x) * 4;
                        block_pixels[py * 4 + px] =
                            ((uint32_t)current_mip_pixels[src_idx + 0] << 0) |
                            ((uint32_t)current_mip_pixels[src_idx + 1] << 8) |
                            ((uint32_t)current_mip_pixels[src_idx + 2] << 16) |
                            ((uint32_t)current_mip_pixels[src_idx + 3] << 24);
                    }
                }
                bc7enc_compress_block(&bc7_buffer[dst_offset], block_pixels, &pack_params);
                dst_offset += 16;
            }
        }

        // Generate the next mip level (if this isn't the last one)
        if (mip < mip_levels - 1) {
            uint32_t next_w          = (mip_w > 1) ? mip_w / 2 : 1;
            uint32_t next_h          = (mip_h > 1) ? mip_h / 2 : 1;
            uint8_t* next_mip_pixels = malloc(next_w * next_h * 4);

            stbir_resize_uint8_linear(
                current_mip_pixels, mip_w, mip_h, 0, next_mip_pixels, next_w, next_h, 0, STBIR_RGBA
            );

            free(current_mip_pixels);
            current_mip_pixels = next_mip_pixels;
            mip_w              = next_w;
            mip_h              = next_h;
        }
    }

    free(current_mip_pixels);

    out_texture->compressed_bytes = bc7_buffer;
    out_texture->total_size       = total_size;
    out_texture->mip_levels       = mip_levels;

    return true;
}
