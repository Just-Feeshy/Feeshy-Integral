#include <uniform_block_state.h>
#include <uniform_manager.h>
#include <memory.h>

#define DEFAULT_INCLUSIVE_BETWEEN_EX_MESSAGE "Value %d is not between %d and %d (inclusive).\n"

// Returns the maximum number of bindings for a given target.
static uint32_t max_gl_bindings(uint32_t target) {
    switch(target) {
        case GL_UNIFORM_BUFFER: {
            int value;
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
            return (uint32_t)value;
        }
        default:
            fprintf(stderr, "Invalid target\n");
            exit(EXIT_FAILURE);
    }
    return 0;
}

// Helper: update the uniform block binding for each pipeline.
static void update_uniform_binding_for_all(const char* name, int binding,
        graphics_pipeline** pipelines, size_t num_pipelines) {
    for (size_t i = 0; i < num_pipelines; i++) {
        set_uniform_block(name, binding, pipelines[i]);
    }
}


// ----------------------
// Uniform Block Object
// ----------------------

static void free_bindings(uniform_block* block, graphics_pipeline** pipelines, size_t num_pipelines) {
    uintptr_t raw_item;
    uintptr_t* item_list = (uintptr_t*)block->bounded_blocks->values;

    FOR_EACH(raw_item, item_list, block->bounded_blocks->length) {
        int binding = (int)(raw_item & ((MAX_GL_BINDINGS) - 1));

        if (contains(block->used_bindings, &binding)) {
            continue;
        }

        uintptr_t item = raw_item & ~((MAX_GL_BINDINGS) - 1);
        unbind_ubo(block, binding, (sized_shader_block*)item, pipelines, num_pipelines);

        delete(block->bounded_blocks, &item);
        block->next_binding = binding;

        MIN_FREE((void*)item, sizeof(sized_shader_block));

        // Adjust cur_addr if needed (assumes cur_addr is globally defined)
        uintptr_t cur_addr = item;
        if (cur_addr < MAX_GL_BINDINGS) {
            cur_addr = MAX_GL_BINDINGS;
        }

        return;
    }

    fprintf(stderr, "Failed to free bindings! Too many shaders bound!\n");
}

void init_ubo(uniform_block* block) {
    // Assume cur_addr is defined elsewhere
    cur_addr = MAX_GL_BINDINGS;
    block->bounded_blocks = init_hash_set();
    block->used_bindings = init_hash_set();
}

void unbind_ubo(uniform_block* block, int binding, sized_shader_block* ssbo,
        graphics_pipeline** pipelines, size_t num_pipelines) {
    unbind_ssbo(ssbo, binding);

    const char* name = block->shader_bindings[binding];
    if (name) {
        switch (ssbo->binding) {
            case GL_UNIFORM_BUFFER:
                update_uniform_binding_for_all(name, binding, pipelines, num_pipelines);
                break;
            default:
                fprintf(stderr, "Invalid binding\n");
                exit(EXIT_FAILURE);
        }
    }

    if (binding < block->next_binding) {
        block->next_binding = binding;
    }
}

void unbind_ubo_just_ssbo(uniform_block* block, sized_shader_block** ssbo,
        graphics_pipeline** pipelines, size_t num_pipelines) {
    uintptr_t item = (uintptr_t)ssbo;
    int binding = (int)(item & ((MAX_GL_BINDINGS) - 1));

    if (contains(block->bounded_blocks, ssbo)) {
        unbind_ubo(block, binding, *ssbo, pipelines, num_pipelines);
        delete(block->bounded_blocks, &item);
    }
}

int bind_ubo(uniform_block* block, sized_shader_block** ssbo,
        graphics_pipeline** pipelines, size_t num_pipelines) {
    int binding = (int)((uintptr_t)(*ssbo) & ((MAX_GL_BINDINGS) - 1));
    sized_shader_block* ssbo_ptr = (sized_shader_block*)((uintptr_t)(*ssbo) & ~((MAX_GL_BINDINGS) - 1));

    if (!contains(block->bounded_blocks, ssbo) && binding == 0) {
        if (block->next_binding >= max_gl_bindings(GL_UNIFORM_BUFFER)) {
            free_bindings(block, pipelines, num_pipelines);
        }

        binding = block->next_binding;
        // Encode the binding into the pointer
        *ssbo = (sized_shader_block*)((uintptr_t)(*ssbo) | (uintptr_t)binding);
        add(block->bounded_blocks, ssbo);

        while (block->shader_bindings[block->next_binding]) {
            block->next_binding++;
        }
    }

    bind_ssbo(ssbo_ptr, binding);
    add(block->used_bindings, &binding);
    return binding;
}

void bind_ubo_with_name(uniform_block* block, const char* name, sized_shader_block** ssbo, graphics_pipeline** pipelines, size_t num_pipelines) {
    sized_shader_block* ssbo_ptr = (sized_shader_block*)((uintptr_t)(*ssbo) & ~((MAX_GL_BINDINGS) - 1));

    int binding = bind_ubo(block, ssbo, pipelines, num_pipelines);
    const char* bound_name = block->shader_bindings[binding];

    if (!bound_name) { // Avoid segfault if not yet set.
        goto set_ubo_block;
    }

    if (strcmp(name, bound_name) != 0) {
set_ubo_block:
        block->shader_bindings[binding] = name;
        switch (ssbo_ptr->binding) {
            case GL_UNIFORM_BUFFER:
                update_uniform_binding_for_all(name, binding, pipelines, num_pipelines);
                break;
            default:
                fprintf(stderr, "Invalid binding\n");
                exit(EXIT_FAILURE);
        }
    }
}

void destroy_ubo(uniform_block* block) {
    if (block->bounded_blocks) {
        free(block->bounded_blocks->values);
    }

    if (block->used_bindings) {
        free(block->used_bindings->values);
    }

    free(block->bounded_blocks);
    free(block->used_bindings);
}


// ------------------------------
// Sized Shader Block Object (SSBO)
// ------------------------------

static void inclusive_between(int min, int max, int value) {
    if (value < min || value > max) {
        fprintf(stderr, DEFAULT_INCLUSIVE_BETWEEN_EX_MESSAGE, value, min, max);
        exit(EXIT_FAILURE);
    }
}

static void serialize(void* data, uint8_t* buffer) {
    memcpy(buffer, data, sizeof(data));
}

sized_shader_block* create_ssbo(uniform_block* ubo, int binding, uint32_t size) {
    sized_shader_block* block = (sized_shader_block*)MIN_ALLOC((void*)cur_addr, sizeof(sized_shader_block));

    if (!block) {
        fprintf(stderr, "Failed to allocate memory for ssbo\n");
        return NULL;
    }

    init_ssbo(block, ubo, binding, size);
    return block;
}

void init_ssbo(sized_shader_block* block, uniform_block* ubo, int binding, uint32_t size) {
    block->binding = binding;
    block->size = size;
    block->data = NULL;
    block->is_dirty = false;
}


void bind_ssbo(sized_shader_block* block, int index) {
    inclusive_between(0, (int)max_gl_bindings(GL_UNIFORM_BUFFER), index);

    if (block->buffer == 0) {
        glGenBuffers(1, &block->buffer);
        glBindBuffer(block->binding, block->buffer);
        glBufferData(block->binding, block->size, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(block->binding, 0);
        block->is_dirty = true;
    }

    if (block->is_dirty) {
        block->is_dirty = false;
        glBindBuffer(block->binding, block->buffer);
        if (block->data) {
            glBufferSubData(block->binding, 0, block->size, block->data);
        } else {
            void* buffer = mem_alloca(block->size);
            stack_allocator allocator;
            stack_init(&allocator, buffer, block->size);
            uint8_t* zero_buffer = stack_calloc(&allocator, block->size, sizeof(uint8_t));
            glBufferSubData(block->binding, 0, block->size, zero_buffer);
        }
        glBindBuffer(block->binding, 0);
    }
    glBindBufferBase(block->binding, index, block->buffer);
}

void unbind_ssbo(sized_shader_block* block, int index) {
    inclusive_between(0, (int)max_gl_bindings(GL_UNIFORM_BUFFER), index);
    glBindBufferBase(block->binding, index, 0);
}

void set_ssbo_data(sized_shader_block* block, void* data, uint32_t size) {
    block->data = data;
    block->size = size;
    block->is_dirty = true;
}

void destroy_ssbo(sized_shader_block* block) {
    unbind_ssbo(block, block->binding);

    if (block->buffer) {
        glDeleteBuffers(1, &block->buffer);
        block->buffer = 0;
    }
}
