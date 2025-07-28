#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <hash_set.h>
#include <hashmap.h>
#include <more_math.h>
#include <pipeline.h>
#include <opengl.h>
#include <utils.h>


/* ==================================================================================
 * DISCLAIMER: This code is REALLY BAD! However, it runs and is not and only runs once!
 * ==================================================================================*/


// I'm not joking, I really don't know what I was thinking when I wrote this.
// This code is a mess, but it works for the purpose of managing uniform blocks in OpenGL.
// Luckily, it does not affect performance at all, especially when it comes to gathering
// research on ray marching optimization techniques.

// ----------------------
// In the future, if I ever stumble upon doing this again, I'm using Rust
// since the Borrow Checker will prevent me from being really stupid.
// I want to throw up every time I look at this code.
// ----------------------

#if defined(_WIN32) || defined(_WIN64)

#define MAX_GL_BINDINGS \
    0x40000000

#else

#define MAX_GL_BINDINGS \
    (1L << ceil_log2(opengl_get_integerv(GL_MAX_UNIFORM_BUFFER_BINDINGS)))

#endif

// Uniform Block Object

typedef struct uniform_block {
    const char* shader_bindings[DEFAULT_HASH_SET_CAPACITY];
    hash_set_t* bounded_blocks;
    hash_set_t* used_bindings;
    int next_binding;
} uniform_block;

// Sized Shader Block Object

static uintptr_t cur_addr;

typedef struct sized_shader_block {
    int binding;
    uint32_t buffer;
    uint32_t size;
    void* data;
    bool is_dirty;
} sized_shader_block;

// Sized Shader Block Methods

sized_shader_block* create_ssbo(uniform_block* ubo, int binding, uint32_t size);
void init_ssbo(sized_shader_block* block, uniform_block* ubo, int binding, uint32_t size);
void bind_ssbo(sized_shader_block* block, int binding);
void unbind_ssbo(sized_shader_block* block, int binding);
void set_ssbo_data(sized_shader_block* block, void* data, uint32_t size);
void destroy_ssbo(sized_shader_block* block);

// Uniform Block Methods (Multi-Pipeline Support)

void init_ubo(uniform_block* block);
void destroy_ubo(uniform_block* block);

void unbind_ubo(uniform_block* block, int binding, sized_shader_block* ssbo, graphics_pipeline** pipelines, size_t pipeline_count);
void unbind_ubo_just_ssbo(uniform_block* block, sized_shader_block** ssbo, graphics_pipeline** pipelines, size_t pipeline_count);

int bind_ubo(uniform_block* block, sized_shader_block** ssbo, graphics_pipeline** pipelines, size_t num_pipeline);
void bind_ubo_with_name(uniform_block* block, const char* name, sized_shader_block** ssbo, graphics_pipeline** pipelines, size_t num_pipeline);
