#pragma once

#include <stdint.h>

typedef struct shader_attribute {
    const char* name;
    int location;
} shader_attribute;

typedef enum shader_type {
    SHADER_VERTEX,
    SHADER_FRAGMENT,
    SHADER_GEOMETRY
} shader_type;

typedef struct shader {
    unsigned* shader;
    unsigned char* source;
    int64_t size;
    uint32_t type;

    uint32_t num_attrs;
    shader_attribute* attributes;
} shader;

void load_shader(const char* path, shader* shader_obj, shader_type type, uint32_t num_attrs, shader_attribute* attributes[num_attrs]);
void compile_shader(shader* shader_obj);
void apply_shader_type(shader* shader_obj, shader_type type);
void destroy_shader(shader* shader_obj);
void destroy_core_shader(shader* shader_obj);
void print_shader_log_info(unsigned int shader);
