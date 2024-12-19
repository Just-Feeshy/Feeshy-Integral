#pragma once

#include <utils.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

#define ALIGN_TO_PAGE(size, page_size) (((size) + (page_size) - 1) & ~((page_size) - 1))


typedef struct stack_allocator {
    uint8_t* base;
    size_t size;
    size_t offset;
} stack_allocator;

void stack_init(stack_allocator* allocator, void* buffer, size_t size) {
    allocator->base = (uint8_t*)buffer;
    allocator->size = size;
    allocator->offset = 0;
}

void* stack_alloc(stack_allocator* allocator, size_t size) {
    if(allocator->offset + size > allocator->size) {
        printf("Stack overflow: Not enough memory in stack allocator!\n");
        return NULL;
    }

    void* result = allocator->base + allocator->offset;
    allocator->offset += size;
    return result;
}

void* stack_calloc(stack_allocator* stack, size_t num_elements, size_t element_size) {
    size_t total_size = num_elements * element_size;

    if (total_size == 0 || stack->offset + total_size > stack->size) {
        printf("Stack overflow or invalid allocation size!\n");
        return NULL;
    }

    void *ptr = stack->base + stack->offset;
    memset(ptr, 0, total_size);
    stack->offset += total_size;

    return ptr;
}

void stack_rewind(stack_allocator* allocator) {
    allocator->offset = 0;
}

// Allocate memory on the stack (Used for spoopy memory management)
void* MIN_ALLOC(void* minimum_address, size_t size) {
    size_t page_size;

#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    page_size = sys_info.dwPageSize;

    // Align size to page boundary
    size = ALIGN_TO_PAGE(size, page_size);

    // Attempt to allocate memory at or above minimum_address
    void* addr = VirtualAlloc(minimum_address, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (addr == NULL) {
        fprintf(stderr, "VirtualAlloc failed with error: %lu\n", GetLastError());
        return NULL;
    }

    // Ensure the returned address meets the minimum address condition
    if ((uintptr_t)addr < (uintptr_t)minimum_address) {
        VirtualFree(addr, 0, MEM_RELEASE);
        return MIN_ALLOC((void*)((uintptr_t)minimum_address + page_size), size);
    }
#else
    // Get system page size
    page_size = getpagesize();

    // Align size to page boundary
    size = ALIGN_TO_PAGE(size, page_size);

    void* addr = minimum_address;

    // Attempt to map memory at or above minimum_address
    void* result = mmap(addr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }

    // Ensure the returned address meets the minimum address condition
    if ((uintptr_t)result < (uintptr_t)minimum_address) {
        munmap(result, size);
        return MIN_ALLOC((void*)((uintptr_t)minimum_address + page_size), size);
    }
    addr = result;
#endif

    return addr;
}

// Free memory allocated by MIN_ALLOC (Used for spoopy memory management)
void MIN_FREE(void* addr, size_t size) {
#if defined(_WIN32) || defined(_WIN64)
    VirtualFree(addr, 0, MEM_RELEASE);
#else
    munmap(addr, size);
#endif
}
