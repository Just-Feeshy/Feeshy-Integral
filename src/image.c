#define STB_IMAGE_IMPLEMENTATION 1
#include <stb_image.h>
#include <texture.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_log.h>
#else
#include <SDL_rwops.h>
#include <SDL_log.h>
#endif

image load_image(const char* path) {
    SDL_RWops* file = SDL_RWFromFile(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        goto cleanup_image;
    }

    int64_t file_size = SDL_RWsize(file);
    uint8_t* file_buffer = (uint8_t*)malloc(file_size);

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

image extract_face(image img, uint32_t face_x, uint32_t face_y) {
    image face_img = {0};

    uint32_t face_size = img.width >> 2;
    uint8_t* face = (uint8_t*)malloc(face_size * face_size * img.channels);

    for(uint32_t y=0; y<face_size; y++) {
        for(uint32_t x=0; x<face_size; x++) {
            uint32_t src_x = face_x * face_size + x;
            uint32_t src_y = face_y * face_size + y;

            for(uint32_t c=0; c<img.channels; c++) {
                face[(y * face_size + x) * img.channels + c] = img.data[(src_y * img.width + src_x) * img.channels + c];
            }
        }
    }
    face_img.data = face;
    face_img.width = face_size;
    face_img.height = face_size;
    face_img.channels = img.channels;

    return face_img;
}
