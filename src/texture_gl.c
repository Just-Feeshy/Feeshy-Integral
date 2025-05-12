#define FNL_IMPL 1

#define STB_IMAGE_IMPLEMENTATION 1
#include <stb_image.h>
#include <texture.h>
#include <opengl.h>
#include <more_math.h>
#include <FastNoiseLite.h>

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
    glBindTexture(GL_TEXTURE_2D, tex->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, tex->image_format, tex->width, tex->height, 0, tex->image_format, GL_UNSIGNED_BYTE, img.data);
    return tex;
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void texture_volume(texture* tex, AABB aabb, float* distanceFieldData, vec3 size) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (unsigned*)&tex->texture);
    glBindTexture(GL_TEXTURE_3D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Allocate memory for the 3D texture
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, size[0], size[1], size[2], 0, GL_RED, GL_FLOAT, NULL);

    // Upload the texture data
    glTexSubImage3D(GL_TEXTURE_3D, 0,
        0, 0, 0,
        size[0], size[1], size[2],
        GL_RED, GL_FLOAT,
        distanceFieldData
    );
}

image* create_simplex_noise(int width, int height) {
    image* img = (image*)malloc(sizeof(image));
    img->width = width;
    img->height = height;
    img->channels = 1;
    img->data = (uint8_t*)malloc(width * height * img->channels);

    fnl_state noise = fnlCreateState();
    noise.seed = 1337;
    noise.octaves = 6;
    noise.frequency = 0.01f;
    noise.noise_type = FNL_NOISE_CELLULAR;
    noise.fractal_type = FNL_FRACTAL_PINGPONG;

    for(int y=0; y<height; y++) {
        for(int x=0; x<width; x++) {
            float n = fnlGetNoise2D(&noise, x, y);
            uint8_t value = (uint8_t)fclamp((n - 0.5f) * -170.0f, 0.0f, 255.0f);
            img->data[y * width + x] = value;
        }
    }

    return img;
}

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
