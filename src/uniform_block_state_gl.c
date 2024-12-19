#include <uniform_block_state.h>
#include <uniform_manager.h>
#include <memory.h>

#define DEFAULT_INCLUSIVE_BETWEEN_EX_MESSAGE "Value %d is not between %d and %d (inclusive).\n"


static uint32_t max_gl_bindings(uint32_t target) {
    switch(target) {
        case GL_UNIFORM_BUFFER: {
            int value;
            glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
        };
        default: fprintf(stderr, "Invalid target\n"); exit(EXIT_FAILURE);
    }

    return 0;
}


// Uniform Block Object

static int int2str_compare(const void* a, const void* b, void* udata) {
    return *(int*)a - *(int*)b;
}

static uint64_t int2str_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const int* str = item;
    return hashmap_sip(str, sizeof(int), seed0, seed1);
}

static void free_bindings(uniform_block* block) {
    size_t iter = 0;
    uintptr_t raw_item;
    uintptr_t* item_list = (uintptr_t*)block->bounded_blocks->values;

    FOR_EACH(raw_item, item_list, block->bounded_blocks->length) {
        int binding = raw_item & ((MAX_GL_BINDINGS) - 1);

        if(contains(block->used_bindings, &binding)) {
            continue;
        }

        uintptr_t item = raw_item & ~((MAX_GL_BINDINGS) - 1);
        unbind_ubo(block, binding, (sized_shader_block*)item);

        //unbind_ubo(block, *(int*)item, *(sized_shader_block**)item);
        //hashmap_delete(block->bounded_blocks, &item);
        delete(block->bounded_blocks, &item);
        block->next_binding = binding;

        MIN_FREE((void*)item, sizeof(sized_shader_block));
        cur_addr = item;
        return;
    }

    fprintf(stderr, "Failed to free bindings! Too many shaders bounded!\n");
}

void init_ubo(uniform_block* block) {
    cur_addr = MAX_GL_BINDINGS;

    block->shader_bindings = hashmap_new(sizeof(int), 0, 0, 0, int2str_hash, int2str_compare, NULL, NULL);
    block->bounded_blocks = init_hash_set();
    block->used_bindings = init_hash_set();
}

void unbind_ubo(uniform_block* block, int binding, sized_shader_block* ssbo) {
    unbind_ssbo(ssbo, binding);

    const char* name = hashmap_get(block->shader_bindings, &binding);

    if(name) {
        switch(ssbo->binding) {
            case GL_UNIFORM_BUFFER: {
                set_uniform_block(name, binding);
            }
            default: fprintf(stderr, "Invalid binding\n"); exit(EXIT_FAILURE);
        }

        hashmap_delete(block->shader_bindings, &binding);
    }

    if(binding < block->next_binding) {
        block->next_binding = binding;
    }
}

int bind_ubo(uniform_block* block, sized_shader_block* ssbo) {
    int binding = (int)((uintptr_t)ssbo & ((MAX_GL_BINDINGS) - 1));

    if(!contains(block->bounded_blocks, &ssbo) && binding == 0) {
        if(block->next_binding >= max_gl_bindings(GL_UNIFORM_BUFFER)) {
            free_bindings(block);
        }

        binding = block->next_binding;
        //hashmap_set(block->bounded_blocks, &ssbo, &binding);
        add(block->bounded_blocks, (void*)((uintptr_t)ssbo | binding));

        while(hashmap_get(block->shader_bindings, &binding)) {
            block->next_binding++;
        }
    }

    bind_ssbo(ssbo, binding);
    add(block->used_bindings, &binding);
    return binding;
}

void bind_ubo_with_name(uniform_block* block, const char* name, sized_shader_block* ssbo) {
    int binding = bind_ubo(block, ssbo);
}


// Sized Shader Block Object

static void inclusive_between(int min, int max, int value) {
    if (value < min || value > max) {
        // Print error message and exit
        fprintf(stderr, DEFAULT_INCLUSIVE_BETWEEN_EX_MESSAGE, value, min, max);
        exit(EXIT_FAILURE);
    }
}

static void serialize(void* data, uint8_t* buffer) {
    memcpy(buffer, data, sizeof(data));
    printf("Serialized data: %d\n", *(int*)buffer);
}

sized_shader_block* create_ssbo(uniform_block* ubo, int binding, uint32_t size) {
    sized_shader_block* block = (sized_shader_block*)MIN_ALLOC((void*)cur_addr, sizeof(sized_shader_block));

    if(!block) {
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
    inclusive_between(0, max_gl_bindings(GL_UNIFORM_BUFFER), index);

    if(!block->buffer) {
        glGenBuffers(1, &block->buffer);
        glBindBuffer(block->binding, block->buffer);
        glBufferData(block->binding, block->size, block->data, GL_DYNAMIC_DRAW);
        glBindBuffer(block->binding, 0);

        block->is_dirty = true;
    }

    if(block->is_dirty) {
        block->is_dirty = false;
        glBindBuffer(block->binding, block->buffer);

        {
            void* buffer = mem_alloca(block->size);
            stack_allocator allocator;
            stack_init(&allocator, buffer, block->size);

            if(block->data) {
                uint8_t* fixed_buffer = stack_alloc(&allocator, block->size);

                if(!fixed_buffer) {
                    fprintf(stderr, "Failed to allocate memory for fixed buffer\n");
                    return;
                }

                serialize(block->data, fixed_buffer);
                stack_rewind(&allocator);
                glBufferSubData(block->binding, 0, block->size, fixed_buffer);
            }else {
                uint8_t* zero_buffer = stack_calloc(&allocator, block->size, sizeof(uint8_t));
                glBufferSubData(block->binding, 0, block->size, zero_buffer);
            }
        }

        glBindBuffer(block->binding, 0);
    }

    glBindBufferBase(block->binding, index, block->buffer);
}

void unbind_ssbo(sized_shader_block* block, int index) {
    inclusive_between(0, max_gl_bindings(GL_UNIFORM_BUFFER), index);
    glBindBufferBase(block->binding, index, 0);
}

void set_ssbo_data(sized_shader_block* block, void* data) {
    block->data = data;
    block->is_dirty = true;
}

void destroy_ssbo(sized_shader_block* block) {
    unbind_ssbo(block, block->binding);

    if(block->buffer) {
        glDeleteBuffers(1, &block->buffer);
        block->buffer = 0;
    }
}
