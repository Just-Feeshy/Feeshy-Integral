#include <shader.h>
#include <util.h>
#include <SDL_rwops.h>
#include <SDL_stdinc.h>
#include <string.h>

// The purpose of goto is to avoid accidental memory leaks.
// Despite on what some developers say, goto is a good practice for memory management.
void load_shader(const char* filename, shader* shader_obj, shader_type type, uint32_t num_attrs, shader_attribute *attributes[num_attrs]) {
    memset(shader_obj, 0, sizeof(shader));

    SDL_RWops *file = SDL_RWFromFile(filename, "rb");
    uint8_t* buffer = NULL;

    if(file == NULL) {
        char out_filename[strlen(filename) + 4]; // Stack based buffer
        strcpy(out_filename, "../");
        strcat(out_filename, filename);

        file = SDL_RWFromFile(out_filename, "rb");
    }

    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    int64_t size = SDL_RWsize(file);
    buffer = (uint8_t*)malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        goto file_error;
    }

    if (size < 0) {
        fprintf(stderr, "Failed to get file size: %s\n", filename);
        goto file_error;
    }

    if (SDL_RWread(file, buffer, 1, size) != size) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        goto file_error;
    }

    shader_obj->source = buffer;
    shader_obj->size = size;
    apply_shader_type(shader_obj, type);

    shader_obj->num_attrs = num_attrs;
    shader_obj->attributes = (shader_attribute*)malloc(num_attrs * sizeof(shader_attribute));

    for (int i = 0; i < num_attrs; i++) {
        shader_obj->attributes[i] = *attributes[i];
    }

file_error:
    SDL_RWclose(file);

    if(!shader_obj->source) {
        free(buffer);
    }
}

void destroy_shader(shader* shader_obj) {
    free(shader_obj->source);
    free(shader_obj->attributes);
    free(shader_obj->shader);

    destroy_core_shader(shader_obj);
}
