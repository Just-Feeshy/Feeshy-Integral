#pragma once

#include <utils.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

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

void* MIN_ALLOC(void* minimum_address, size_t size) {
    fprintf(stderr, "MIN_ALLOC called: minimum_address=%p, size=%zu\n", minimum_address, size);

    if (size == 0) {
        fprintf(stderr, "MIN_ALLOC error: size cannot be 0\n");
        return NULL;
    }

    size_t page_size;

#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    page_size = sys_info.dwPageSize;

    if (minimum_address != NULL && ((uintptr_t)minimum_address % page_size) != 0) {
        minimum_address = (void*)(((uintptr_t)minimum_address + page_size - 1) & ~(page_size - 1));
    }

    size = ALIGN_TO_PAGE(size, page_size);

    if (minimum_address != NULL) {
        uintptr_t min_addr = (uintptr_t)minimum_address;
        if (min_addr > UINTPTR_MAX - size) {
            fprintf(stderr, "MIN_ALLOC error: address + size would overflow\n");
            return NULL;
        }
    }

    void* addr = VirtualAlloc(minimum_address, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (addr == NULL) {
        DWORD error = GetLastError();
        fprintf(stderr, "VirtualAlloc failed with error: %lu\n", error);

        switch (error) {
            case ERROR_INVALID_PARAMETER:
                fprintf(stderr, "Invalid parameter - check size (%zu) and address (%p)\n", size, minimum_address);
                break;
            case ERROR_NOT_ENOUGH_MEMORY:
                fprintf(stderr, "Not enough memory available\n");
                break;
            case ERROR_INVALID_ADDRESS:
                fprintf(stderr, "Invalid memory address\n");
                break;
            default:
                fprintf(stderr, "Unknown VirtualAlloc error\n");
                break;
        }
        return NULL;
    }

    if (minimum_address != NULL && (uintptr_t)addr < (uintptr_t)minimum_address) {
        VirtualFree(addr, 0, MEM_RELEASE);
        static int retry_count = 0;
        if (retry_count < 10) {
            retry_count++;
            void* result = MIN_ALLOC((void*)((uintptr_t)minimum_address + page_size), size);
            retry_count--;
            return result;
        } else {
            fprintf(stderr, "MIN_ALLOC: Too many retries, giving up\n");
            return NULL;
        }
    }
#else
    // Get system page size
    #if defined(_SC_PAGESIZE)
    page_size = getpagesize();
    #elif defined(_SC_PAGE_SIZE)
    page_size = sysconf(_SC_PAGE_SIZE);
    #else
    page_size = 4096;
    #endif

    size = ALIGN_TO_PAGE(size, page_size);

    void* addr = minimum_address;

    #ifdef EMSCRIPTEN
    void* result = sbrk(size);

    if(result == (void*)-1) {
        perror("sbrk failed: OUT OF MEMORY\n");
        return NULL;
    }
    #else
    void* result = mmap(minimum_address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }
    #endif

    if (minimum_address != NULL && (uintptr_t)result < (uintptr_t)minimum_address) {
        munmap(result, size);
        static int retry_count = 0;
        if (retry_count < 10) {
            retry_count++;
            void* retry_result = MIN_ALLOC((void*)((uintptr_t)minimum_address + page_size), size);
            retry_count--;
            return retry_result;
        } else {
            fprintf(stderr, "MIN_ALLOC: Too many retries, giving up\n");
            return NULL;
        }
    }
    addr = result;
#endif

    return addr;
}

void MIN_FREE(void* addr, size_t size) {
    if (addr == NULL) {
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (!VirtualFree(addr, 0, MEM_RELEASE)) {
        DWORD error = GetLastError();
        fprintf(stderr, "VirtualFree failed with error: %lu\n", error);
    }
#else
    if (munmap(addr, size) == -1) {
        perror("munmap failed");
    }
#endif
}

void mem_copy(void* dest, void* src, size_t size) {
    if (dest == NULL || src == NULL || size == 0) {
        return;
    }

    uint8_t* _dest = (uint8_t*)dest;
    const uint8_t* _end = _dest + size;
    const uint8_t* _src = (const uint8_t*)src;

    while (_dest != _end) {
        *_dest++ = *_src++;
    }
}
