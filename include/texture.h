#pragma once

#include <stdint.h>
#include <aabb.h>
#include <cglm/vec3.h>
#include <mesh.h>

typedef struct texture {
    int width;
    int height;
    int depth;
    unsigned image_format;
    unsigned texture;
    unsigned type;
} texture;

typedef struct image {
    int width;
    int height;
    int channels;
    uint8_t* data;
} image;

texture texture_red_init();
texture* texture_init(image img);
void texture_bind(texture* tex, unsigned unit);
image* create_simplex_noise(int width, int height);
image load_image(const char* path);
image load_image_raw(const uint8_t* data, uint32_t size);

#ifndef EMSCRIPTEN
#include <desktop/parallelism.h>

texture* texture_volume(struct GPU_MODULE* modul, Mesh* mesh, AABB* aabb);
#endif
