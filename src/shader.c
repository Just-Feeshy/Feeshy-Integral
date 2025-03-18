#include <shader.h>
#include <utils.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_stdinc.h>
#else
#include <SDL_rwops.h>
#include <SDL_stdinc.h>
#endif

#include <string.h>

// The purpose of goto is to avoid accidental memory leaks.
// Despite on what some developers say, goto is a good practice for memory management.
void load_shader(const char* filename, shader* shader_obj, shader_type type, uint32_t num_attrs, shader_attribute *attributes[num_attrs]) {
    memset(shader_obj, 0, sizeof(shader));

    SDL_RWops *file = SDL_RWFromFile(filename, "rb");
    char* buffer = NULL;

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

    buffer = (char*)malloc(32);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        goto file_error;
    }


    snprintf(buffer, 32, "#version %d%d%d %s\n\n", GL_APP_MAJOR_VERSION, GL_APP_MINOR_VERSION, 0, (GL_APP_PROFILE_MASK == SDL_GL_CONTEXT_PROFILE_ES) ? "es" : "core");

    int64_t file_size = SDL_RWsize(file);
    int64_t size = file_size + strlen(buffer) + 1;
    buffer = (char*)realloc(buffer, size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        goto file_error;
    }

    if (size == -1) {
        fprintf(stderr, "Failed to get file size: %s\n", filename);
        goto file_error;
    }

    if (SDL_RWread(file, buffer + strlen(buffer), 1, file_size) != file_size) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        goto file_error;
    }

    // Sanatize the shader source (Reverse order)
    for (char* p = buffer + size - 1; p >= buffer; --p) {
        if (*p < 32 && *p != '\n' && *p != '\t') {
            *p = ' ';
            break;
        }
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
