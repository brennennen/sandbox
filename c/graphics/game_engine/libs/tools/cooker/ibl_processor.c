
#include <math.h>
#include <string.h>

#include "ibl_processor.h"
#include "libs/engine/core/logger.h"
#include "stb_image.h"

#include "shared/pak_format.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static size_t calculate_cubemap_mip_chain_size(
    uint32_t base_size,
    uint32_t mip_levels,
    size_t   bytes_per_pixel
) {
    size_t   total_size   = 0;
    uint32_t current_size = base_size;

    for (uint32_t i = 0; i < mip_levels; i++) {
        size_t face_area     = (size_t)current_size * (size_t)current_size;
        size_t mip_byte_size = face_area * 6 * bytes_per_pixel;

        total_size += mip_byte_size;

        if (current_size > 1) {
            current_size /= 2;
        }
    }

    return total_size;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static vec3_t get_cubemap_direction(int face, float u, float v) {
    // u and v range from -1.0 to 1.0
    switch (face) {
    case 0:
        return (vec3_t){1.0f, -v, -u}; // +X (Right)
        // return (vec3_t){1.0f, v, -u};
    case 1:
        return (vec3_t){-1.0f, -v, u}; // -X (Left)
    case 2:
        return (vec3_t){u, 1.0f, v}; // +Y (Top)
    case 3:
        return (vec3_t){u, -1.0f, -v}; // -Y (Bottom)
    case 4:
        return (vec3_t){u, -v, 1.0f}; // +Z (Front)
    case 5:
        return (vec3_t){-u, -v, -1.0f}; // -Z (Back)
    default:
        return (vec3_t){0, 0, 0};
    }
}

static void sample_equirectangular(
    const float* pixels,
    int          width,
    int          height,
    vec3_t       dir,
    float*       out_rgba
) {
    float  len      = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    vec3_t norm_dir = {dir.x / len, dir.y / len, dir.z / len};

    float phi   = atan2f(norm_dir.z, norm_dir.x);
    float theta = asinf(norm_dir.y);

    float u = (phi / (2.0f * M_PI)) + 0.5f;
    float v = (theta / M_PI) + 0.5f;

    float x = u * (width - 1);
    float y = v * (height - 1);

    int x0 = (int)floorf(x);
    int y0 = (int)floorf(y);
    int x1 = (x0 + 1 < width) ? x0 + 1 : x0;
    int y1 = (y0 + 1 < height) ? y0 + 1 : y0;

    float tx = x - x0;
    float ty = y - y0;

    const float* p00 = &pixels[(y0 * width + x0) * 4];
    const float* p10 = &pixels[(y0 * width + x1) * 4];
    const float* p01 = &pixels[(y1 * width + x0) * 4];
    const float* p11 = &pixels[(y1 * width + x1) * 4];

    for (int i = 0; i < 4; i++) {
        float top    = p00[i] * (1.0f - tx) + p10[i] * tx;
        float bottom = p01[i] * (1.0f - tx) + p11[i] * tx;
        out_rgba[i]  = top * (1.0f - ty) + bottom * ty;
    }
}

bool process_ibl_textures(
    arena_t*           pak_arena,
    arena_t*           scratch_arena,
    const char*        hdri_path,
    environment_pak_t* out_pak_env
) {
    log_info("Starting IBL Processing for: %s", hdri_path);
    int    width, height, channels;
    float* raw_hdr_pixels = stbi_loadf(hdri_path, &width, &height, &channels, 4);
    if (!raw_hdr_pixels) {
        log_error("Failed to load HDRI: %s", stbi_failure_reason());
        return false;
    }
    arena_temp_t scratch_mark = arena_begin_temp(scratch_arena);

    // equirectangul to cubemap
    uint32_t cube_size      = 1024;
    size_t   cube_byte_size = cube_size * cube_size * 6 * 4 * sizeof(float);
    float*   temp_cubemap   = arena_push_array(scratch_arena, float, cube_size * cube_size * 6 * 4);

    // TODO: Map raw_hdr_pixels (equirectangular) to temp_cubemap (6 faces).
    for (int face = 0; face < 6; face++) {
        for (uint32_t y = 0; y < cube_size; y++) {
            for (uint32_t x = 0; x < cube_size; x++) {
                float  u         = ((x + 0.5f) / (float)cube_size) * 2.0f - 1.0f;
                float  v         = ((y + 0.5f) / (float)cube_size) * 2.0f - 1.0f;
                vec3_t dir       = get_cubemap_direction(face, u, v);
                size_t pixel_idx = (face * cube_size * cube_size) + (y * cube_size) + x;
                sample_equirectangular(
                    raw_hdr_pixels, width, height, dir, &temp_cubemap[pixel_idx * 4]
                );
            }
        }
    }

    float* debug_ptr = (float*)temp_cubemap;
    float  sum       = 0.0f;
    for (int i = 0; i < 100; i++) {
        sum += debug_ptr[i];
    }
    log_info("Cooker Debug: Pixel sum (first 100 floats): %f", sum);

    if (isnan(sum)) {
        log_error("CRITICAL: Cooker generated NaNs!");
    }

    out_pak_env->skybox_cubemap.width     = cube_size;
    out_pak_env->skybox_cubemap.height    = cube_size;
    out_pak_env->skybox_cubemap.channels  = 4;
    out_pak_env->skybox_cubemap.format    = PAK_TEX_FORMAT_RGBA32F;
    out_pak_env->skybox_cubemap.data_size = cube_byte_size;

    float total_val = 0.0f;
    for (size_t i = 0; i < (cube_size * cube_size * 6 * 4); i++) {
        total_val += temp_cubemap[i];
    }
    log_info("Cooker Debug: Total pixel sum of cubemap: %f", total_val);

    if (total_val == 0.0f) {
        log_error("CRITICAL: Cubemap generation resulted in an all-black texture!");
    }

    // log_info("Dumping Face 0 to debug_face_0.hdr...");
    // stbi_write_hdr("debug_face_0.hdr", cube_size, cube_size, 4, temp_cubemap);

    // log_info("Dumping Face 1 to debug_face_1.hdr...");
    // float* face_1_ptr = temp_cubemap + (cube_size * cube_size * 4);
    // stbi_write_hdr("debug_face_1.hdr", cube_size, cube_size, 4, face_1_ptr);

    void* skybox_dest                       = arena_push_aligned(pak_arena, cube_byte_size, 16);
    out_pak_env->skybox_cubemap.data_offset = arena_get_offset_of(pak_arena, skybox_dest);
    memcpy(skybox_dest, temp_cubemap, cube_byte_size);

    // irradiance map
    uint32_t irr_size        = 32;
    size_t   irr_byte_size   = irr_size * irr_size * 6 * 4 * sizeof(float);
    float*   temp_irradiance = arena_push_array(scratch_arena, float, irr_size * irr_size * 6 * 4);
    // TODO: Convolve temp_cubemap to generate temp_irradiance
    out_pak_env->irradiance_map.width     = irr_size;
    out_pak_env->irradiance_map.height    = irr_size;
    out_pak_env->irradiance_map.channels  = 4;
    out_pak_env->irradiance_map.format    = PAK_TEX_FORMAT_RGBA32F;
    out_pak_env->irradiance_map.data_size = irr_byte_size;

    void* irr_dest                          = arena_push_aligned(pak_arena, irr_byte_size, 16);
    out_pak_env->irradiance_map.data_offset = arena_get_offset_of(pak_arena, irr_dest);
    memcpy(irr_dest, temp_irradiance, irr_byte_size);

    // pre-filtered spec mips
    uint32_t pref_size       = 128; // Usually smaller than the main skybox
    uint32_t mip_levels      = 5;
    size_t   pref_total_size = calculate_cubemap_mip_chain_size(
        pref_size, mip_levels, sizeof(float) * 4
    );
    float* temp_prefiltered = arena_push_aligned(scratch_arena, pref_total_size, 16);
    // TODO: Run specular convolution based on roughness values
    out_pak_env->prefiltered_env_map.width     = pref_size;
    out_pak_env->prefiltered_env_map.height    = pref_size;
    out_pak_env->prefiltered_env_map.channels  = 4;
    out_pak_env->prefiltered_env_map.format    = PAK_TEX_FORMAT_RGBA32F;
    out_pak_env->prefiltered_env_map.data_size = pref_total_size;
    void* pref_dest                            = arena_push_aligned(pak_arena, pref_total_size, 16);
    out_pak_env->prefiltered_env_map.data_offset = arena_get_offset_of(pak_arena, pref_dest);
    memcpy(pref_dest, temp_prefiltered, pref_total_size);

    stbi_image_free(raw_hdr_pixels);
    arena_end_temp(scratch_arena, scratch_mark);

    log_info("IBL Processing Complete.");
    return true;
}
