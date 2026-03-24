#ifndef ASSETS_IMAGE_H
#define ASSETS_IMAGE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    void*    pixels;
    size_t   size;
} image_t;

bool image_load(const char* path, image_t* out_image);
void image_free(image_t* image);

image_t image_create_placeholder();
image_t image_create_checkerboard(uint32_t w, uint32_t h);

#endif