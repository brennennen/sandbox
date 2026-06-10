
#include "bc7/bc7enc.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "libs/core/resources/image.h"

#include "libs/core/arena.h"
#include "libs/core/string_span.h"

float* image_loadf(
    arena_t*      arena,
    string_span_t path,
    int*          x,
    int*          y,
    int*          channels_in_file,
    int           desired_channels
) {
    char* path_cstr = span_to_arena_cstr(path, arena);
    return stbi_loadf(path_cstr, x, y, channels_in_file, 4);
}

const char* image_failure_reason() { return stbi_failure_reason(); }

int image_info_from_memory(uint8_t const* buffer, int len, int* x, int* y, int* comp) {
    return stbi_info_from_memory(buffer, len, x, y, comp);
}

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
) {
    return stbir_resize_uint8_linear(
        input_pixels,
        input_w,
        input_h,
        input_stride_in_bytes,
        output_pixels,
        output_w,
        output_h,
        output_stride_in_bytes,
        (stbir_pixel_layout)pixel_type
    );
}

uint8_t* image_load_from_memory(
    uint8_t const* buffer,
    int            len,
    int*           x,
    int*           y,
    int*           channels_in_file,
    int            desired_channels
) {
    return stbi_load_from_memory(buffer, len, x, y, channels_in_file, desired_channels);
}

bool image_load_from_memory2(const void* buffer, int len, image_t* out_image) {
    out_image->pixels = stbi_load_from_memory(
        (const stbi_uc*)buffer,
        len,
        &out_image->width,
        &out_image->height,
        &out_image->channels,
        STBI_rgb_alpha
    );

    out_image->channels = 4;
    return out_image->pixels != NULL;
}

void image_free(void* retval_from_stbi_load) { stbi_image_free(retval_from_stbi_load); }

void image_free2(image_t* image) {
    if (image->pixels) {
        free(image->pixels);
        image->pixels = NULL;
    }
}
