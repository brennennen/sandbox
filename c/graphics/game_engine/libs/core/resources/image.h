#pragma once

#include "libs/core/arena.h"
#include "libs/core/string_span.h"

typedef enum {
    IMAGE_1CHANNEL = 1,
    IMAGE_2CHANNEL = 2,
    IMAGE_RGB      = 3, // 3-chan, with order specified (for channel flipping)
    IMAGE_BGR      = 0, // 3-chan, with order specified (for channel flipping)
    IMAGE_4CHANNEL = 5,

    IMAGE_RGBA = 4, // alpha formats, where alpha is NOT premultiplied into color channels
    IMAGE_BGRA = 6,
    IMAGE_ARGB = 7,
    IMAGE_ABGR = 8,
    IMAGE_RA   = 9,
    IMAGE_AR   = 10,

    IMAGE_RGBA_PM = 11, // alpha formats, where alpha is premultiplied into color channels
    IMAGE_BGRA_PM = 12,
    IMAGE_ARGB_PM = 13,
    IMAGE_ABGR_PM = 14,
    IMAGE_RA_PM   = 15,
    IMAGE_AR_PM   = 16,

    IMAGE_RGBA_NO_AW = 11, // alpha formats, where NO alpha weighting is applied at all!
    IMAGE_BGRA_NO_AW = 12, //   these are just synonyms for the _PM flags (which also do
    IMAGE_ARGB_NO_AW = 13, //   no alpha weighting). These names just make it more clear
    IMAGE_ABGR_NO_AW = 14, //   for some folks).
    IMAGE_RA_NO_AW   = 15,
    IMAGE_AR_NO_AW   = 16,
} image_pixel_layout;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    void*    pixels;
    size_t   size;
    uint32_t mip_levels;
    bool     is_cubemap;
} image_t;

float* image_loadf(
    arena_t*      arena,
    string_span_t path,
    int*          x,
    int*          y,
    int*          channels_in_file,
    int           desired_channels
);

const char* image_failure_reason();

int image_info_from_memory(uint8_t const* buffer, int len, int* x, int* y, int* comp);

uint8_t* image_resize_uint8_linear(
    const unsigned char* input_pixels,
    int                  input_w,
    int                  input_h,
    int                  input_stride_in_bytes,
    unsigned char*       output_pixels,
    int                  output_w,
    int                  output_h,
    int                  output_stride_in_bytes,
    image_pixel_layout   pixel_type
);

uint8_t* image_load_from_memory(
    uint8_t const* buffer,
    int            len,
    int*           x,
    int*           y,
    int*           channels_in_file,
    int            desired_channels
);

bool image_load_from_memory2(const void* buffer, int len, image_t* out_image);

void image_free(void* retval_from_stbi_load);

void image_free2(image_t* image);
