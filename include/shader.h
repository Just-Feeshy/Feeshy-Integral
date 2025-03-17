#pragma once

#include <stdint.h>
#include <opengl.h>

typedef struct shader_attribute {
    const char* name;
    unsigned location;
} shader_attribute;

typedef enum shader_type {
    SHADER_VERTEX,
    SHADER_FRAGMENT,
} shader_type;

typedef struct shader {
    unsigned* shader;
    char* source;
    int64_t size;
    uint32_t type;

    uint32_t num_attrs;
    shader_attribute* attributes;
} shader;

void load_shader(const char* filename, shader* shader_obj, shader_type type, uint32_t num_attrs, shader_attribute *attributes[num_attrs]);
void compile_shader(shader* shader_obj);
void apply_shader_type(shader* shader_obj, shader_type type);
void destroy_shader(shader* shader_obj);
void destroy_core_shader(shader* shader_obj);
void print_shader_log_info(unsigned int shader);
