#pragma once

#include <stdint.h>

typedef struct texture {
    int width;
    int height;
    int depth;
    unsigned image_format;
    uintptr_t texture;
} texture;

void texture_init(texture* tex, int width, int height, unsigned channels, uint8_t* data);
void texture_bind(texture* tex, unsigned unit);
