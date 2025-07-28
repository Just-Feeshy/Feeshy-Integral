#pragma once

#define FOR_EACH(item, array, size) \
    for (unsigned __i = 0; __i < size && ((item = array[__i]), 1); __i++)

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#include <malloc.h>

// Use smaller stack allocations on Windows to prevent overflow
#define mem_alloca(size) ((size) > 8192 ? malloc(size) : _alloca(size))
#define mem_free_alloca(ptr, size) do { if ((size) > 8192) free(ptr); } while(0)
#define FORCE_INLINE __forceinline

static FORCE_INLINE int __builtin_ctz(unsigned x)
{
#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)
    return (int)_CountTrailingZeros(x);
#elif defined(__AVX2__) || defined(__BMI__)
    return (int)_tzcnt_u32(x);
#else
    unsigned long r;
    _BitScanForward(&r, x);
    return (int)r;
#endif
}

#elif !defined(__MSC_VER) || defined(__clang__)
#include <stdio.h>
#include <alloca.h>

#define FORCE_INLINE __attribute__((always_inline))
#define mem_alloca(size) alloca(size)
#define mem_free_alloca(ptr, size) do { } while(0)
#endif

#include <config.h>


// There can be multiple cases where geometry pass is used

#ifndef EMSCRIPTEN
#define USE_OPENCL

#if FRAGMENT_SELECTOR == 2
#define USE_DFAO
// #define HAS_GEOMETRY_PASS
#endif

#else

#if FRAGMENT_SELECTOR == 2
#undef FRAGMENT_SELECTOR
#define FRAGMENT_SELECTOR 0
#warning "Fragment Selector 2 is not supported in Emscripten due to OpenCL not being available. Falling back to Fragment Selector 0."

#endif

#endif
