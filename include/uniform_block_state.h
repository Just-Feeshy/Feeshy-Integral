#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <hash_set.h>
#include <hashmap.h>
#include <more_math.h>
#include <opengl.h>
#include <utils.h>

#define MAX_GL_BINDINGS \
    1L << ceil_log2(opengl_get_integerv(GL_MAX_UNIFORM_BUFFER_BINDINGS))

// Uniform Block Object

typedef struct uniform_block {
    struct hashmap* shader_bindings;
    hash_set_t* bounded_blocks;
    hash_set_t* used_bindings;
    int next_binding;
} uniform_block;


// Sized Shader Block Object

uintptr_t cur_addr;

typedef struct sized_shader_block {
    uniform_block ubo;
    int binding;
    uint32_t buffer;
    uint32_t size;
    void* data;
    bool is_dirty;
} sized_shader_block;

sized_shader_block* create_ssbo(uniform_block* ubo, int binding, uint32_t size);
void init_ssbo(sized_shader_block* block, uniform_block* ubo, int binding, uint32_t size);
void bind_ssbo(sized_shader_block* block, int binding);
void unbind_ssbo(sized_shader_block* block, int binding);
void set_ssbo_data(sized_shader_block* block, void* data);
void destroy_ssbo(sized_shader_block* block);


// Uniform Block Object Methods

void init_ubo(uniform_block* block);
void unbind_ubo(uniform_block* block, int binding, sized_shader_block* ssbo);
int bind_ubo(uniform_block* block, sized_shader_block* ssbo);
void bind_ubo_with_name(uniform_block* block, const char* name, sized_shader_block* ssbo);
