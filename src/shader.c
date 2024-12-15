#include <shader.h>
#include <util.h>
#include <SDL_rwops.h>
#include <SDL_stdinc.h>

// The purpose of goto is to avoid accidental memory leaks.
// Despite on what some developers say, goto is a good practice for memory management.
void load_shader(const char* filename, shader* shader_obj, shader_type type, uint32_t num_attrs, shader_attribute *attributes[num_attrs]) {
    memset(shader_obj, 0, sizeof(shader));

    SDL_RWops *file = SDL_RWFromFile(filename, "rb");
    uint8_t* buffer = NULL;

    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        goto file_error;
    }

    int64_t size = SDL_RWsize(file);
    buffer = (uint8_t*)malloc(size);

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

    shader_obj->num_attrs = 0;
    shader_obj->attributes = NULL;

file_error:
    SDL_RWclose(file);

    if(!shader_obj->source) {
        free(buffer);
    }
}

void destroy_shader(shader* shader_obj) {
    free(shader_obj->source);
    destroy_core_shader(shader_obj);
}