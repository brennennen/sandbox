#define STB_IMAGE_IMPLEMENTATION
#include "image.h"
#include "engine/core/logger.h"
#include "stb_image.h"

bool image_load(const char* path, image_t* out_image) {
    int w;
    int h;
    int c;

    unsigned char* data = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);

    if (!data) {
        log_error("assets/image: failed to load %s", path);
        return false;
    }

    out_image->width    = (uint32_t)w;
    out_image->height   = (uint32_t)h;
    out_image->channels = 4;
    out_image->pixels   = data;
    out_image->size     = (size_t)w * h * 4;

    return true;
}

image_t image_create_placeholder() {
    image_t img = {
        .width    = 2,
        .height   = 2,
        .channels = 4,
        .size     = 2 * 2 * 4,
    };

    uint32_t* pixels = malloc(img.size);
    // Magenta and Black checkerboard
    pixels[0] = 0xFFFF00FF;
    pixels[1] = 0xFF000000;
    pixels[2] = 0xFF000000;
    pixels[3] = 0xFFFF00FF;

    img.pixels = pixels;
    return img;
}

void image_free(image_t* image) {
    if (image->pixels) {
        free(image->pixels);
        image->pixels = NULL;
    }
}

bool image_load_from_memory(const void* buffer, int len, image_t* out_image) {
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
