#define FNL_IMPL 1

#define STB_IMAGE_IMPLEMENTATION 1
#include <stb_image.h>
#include <texture.h>
#include <opengl.h>
#include <more_math.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_rwops.h>
#endif

#define K1 0.366025404 // (sqrt(3)-1)/2;
#define K2 0.211324865 // (3-sqrt(3))/6;

texture* texture_init(image img) {
    texture* tex = (texture*)malloc(sizeof(texture));
    tex->width = img.width;
    tex->height = img.height;
    tex->type = GL_TEXTURE_2D;
    tex->depth = 1;

    switch(img.channels) {
        case 1: tex->image_format = GL_RED; break;
        case 2: tex->image_format = GL_RG; break;
        case 3: tex->image_format = GL_RGB; break;
        case 4: tex->image_format = GL_RGBA; break;
        default: fprintf(stderr, "Invalid number of channels\n"); exit(EXIT_FAILURE);
    }

    if(img.data == NULL) {
        fprintf(stderr, "Invalid data\n");
        exit(EXIT_FAILURE);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex->texture);
    glBindTexture(tex->type, tex->texture);
    glTexParameteri(tex->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(tex->type, 0, tex->image_format, tex->width, tex->height, 0, tex->image_format, GL_UNSIGNED_BYTE, img.data);
    glBindTexture(tex->type, 0);
    return tex;
}

texture texture_red_init() {
    texture tex = {0};
    tex.width = 1;
    tex.height = 1;
    tex.depth = 1;
    tex.image_format = GL_RGBA;
    tex.type = GL_TEXTURE_2D;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex.texture);
    glBindTexture(tex.type, tex.texture);
    glTexParameteri(tex.type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(tex.type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLubyte red_pixel[4] = { 255, 0, 0, 255 };
    glTexImage2D(tex.type, 0, tex.image_format, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, red_pixel);
    glBindTexture(tex.type, 0);
    return tex;
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(tex->type, (unsigned)tex->texture);
    glTexParameteri(tex->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

#ifndef EMSCRIPTEN
texture* texture_volume(struct GPU_MODULE* modul, Mesh* mesh, AABB* aabb) {
    texture* tex = (texture*)malloc(sizeof(texture));
    tex->width = UNIFORM_GRID_X;
    tex->height = UNIFORM_GRID_Y;
    tex->depth = UNIFORM_GRID_Z;
    tex->type = GL_TEXTURE_3D;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (unsigned*)&tex->texture);
    glBindTexture(tex->type, (unsigned)tex->texture);

    size_t data_size = 5;
    GPU_MEM* data = malloc(sizeof(GPU_MEM) * data_size);

    float* buffer = NULL;

    // This is stupid, but it works
    // For anyone reading this, please don't do this.
    // This is a hack to get around OpenGL with OpenCL not supported on M1
    // To explain how and why this is a really really bad idea:

    // 1. Send memory from RAM to GPU
    // 2. Send output from GPU to RAM instead of VRAM (The more logical approach)
    // 3. Send output from RAM to VRAM but now as a texture
    // 4. Now have the GPU read the RAM which from step 2 it already computed
    // 5. Hate yourself for doing this

    parallelism_alloc_MDF(modul, aabb, mesh, (int32_t)tex->texture, data, UNIFORM_GRID_SIZE);
    parallelism_invoke_MDF(modul, data[0], &buffer, UNIFORM_GRID_SIZE);

    // for(size_t i=0; i<UNIFORM_GRID_SIZE; i++) {
    //     if(buffer[i] > 1.0f) {
    //         continue;
    //     }

    //     printf("%f ", buffer[i]);
    // }

    glTexImage3D(tex->type, 0, GL_R32F, tex->width, tex->height, tex->depth, 0, GL_RED, GL_FLOAT, buffer);
    glTexParameteri(tex->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(tex->type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(tex->type, 0);
    parallelism_clean(modul, data, data_size);
    return tex;
}
#endif

image load_image(const char* path) {
    SDL_RWops* file = SDL_RWFromFile(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        goto cleanup_image;
    }

    int64_t file_size = SDL_RWsize(file);
    uint8_t* file_buffer = (uint8_t*)SDL_malloc(file_size);

    if (file_buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", path);
        goto cleanup_image;
    }

    if(SDL_RWread(file, file_buffer, 1, file_size) != file_size) {
        fprintf(stderr, "Failed to read file: %s\n", path);
        goto cleanup_image;
    }

    image img = {0};
    img.data = stbi_load_from_memory(file_buffer, file_size, &img.width, &img.height, &img.channels, 0);

    if(img.data != NULL) {
        SDL_Log("Image loaded successfully (%dx%d, %d channels)\n", img.width, img.height, img.channels);
    } else {
        SDL_Log("Failed to load image file %s\n", path);
    }

cleanup_image:
    if(file) {
        SDL_RWclose(file);
    }

    if(file_buffer) {
        free(file_buffer);
    }

    return img;
}

image load_image_raw(const uint8_t* data, uint32_t size) {
    image img = {0};
    img.data = stbi_load_from_memory(data, size, &img.width, &img.height, &img.channels, 0);

    if(img.data != NULL) {
        SDL_Log("Image loaded successfully (%dx%d, %d channels)\n", img.width, img.height, img.channels);
    } else {
        SDL_Log("Failed to load image data\n");
    }

    return img;
}
