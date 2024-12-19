#include <uniform_manager.h>
#include <hashmap.h>
#include <string.h>

static struct hashmap* uniform_map;

int uniform_compare(const void *a, const void *b, void *udata) {
    const constant_location* la = a;
    const constant_location* lb = b;
    return strcmp(la->name, lb->name);
}

static uint64_t uniform_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const constant_location* loc = item;
    return hashmap_sip(loc->name, strlen(loc->name), seed0, seed1);
}

static constant_location get_constant_location(graphics_pipeline* pipe, const char* name) {
    constant_location loc;
    loc.name = name;
    loc.core.location = glGetUniformLocation(pipe->pipeline_core.programId, name);
    loc.core.type = GL_FLOAT;

    int count = 0;
    glGetProgramiv(pipe->pipeline_core.programId, GL_ACTIVE_UNIFORMS, &count);

    char array_name[1024];
    strcpy(array_name, name);
    strcat(array_name, "[0]");

    for (int i = 0; i < count; i++) {
        GLenum type;
        char uniform_name[1024];
        GLsizei length;
        int size;

        glGetActiveUniform(pipe->pipeline_core.programId, i, 1024 - 1, &length, &size, &type, uniform_name);
        if(strcmp(uniform_name, name) == 0 || strcmp(uniform_name, array_name) == 0) {
            loc.core.type = type;
            break;
        }
    }

    if (loc.core.location == -1) {
        fprintf(stderr, "Failed to get uniform location for %s\n", name);
    }

    return loc;
}

void uniform_manager_init() {

    // Initialize our structures
    memset(&uniform_map, 0, sizeof(uniform_map));

    // Create the hash maps
    uniform_map = hashmap_new(sizeof(constant_location), 0, 0, 0, uniform_hash, uniform_compare, NULL, NULL);
}

void create_constant_location(graphics_pipeline* pipe, const char* name) {
    constant_location loc = get_constant_location(pipe, name);
    hashmap_set(uniform_map, &loc);
}

void set_uniform_int(const char* name, int value) {
    const constant_location* loc;

    if ((loc = hashmap_get(uniform_map, &(constant_location){.name=name})) == NULL) {
        fprintf(stderr, "Failed to find uniform %s\n", name);
        return;
    }

    glUniform1i(loc->core.location, value);
}

void set_uniform_vec2(const char* name, float x, float y) {
    const constant_location* loc;

    if ((loc = hashmap_get(uniform_map, &(constant_location){.name=name})) == NULL) {
        fprintf(stderr, "Failed to find uniform %s\n", name);
        return;
    }

    glUniform2f(loc->core.location, x, y);
}

void set_uniform_block(const char* name, unsigned int binding) {
    const constant_location* loc;

    if ((loc = hashmap_get(uniform_map, &(constant_location){.name=name})) == NULL) {
        fprintf(stderr, "Failed to find uniform %s\n", name);
        return;
    }

    glUniformBlockBinding(loc->core.location, binding, loc->core.location);
}
