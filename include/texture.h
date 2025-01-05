#pragma once

#include <stdint.h>

typedef struct texture {
    int width;
    int height;
    int depth;
    unsigned image_format;
    uintptr_t texture;
} texture;

typedef struct image {
    int width;
    int height;
    int channels;
    uint8_t* data;
} image;

void texture_init(texture* tex, image* img);
void texture_bind(texture* tex, unsigned unit);
image* create_simplex_noise(int width, int height);
