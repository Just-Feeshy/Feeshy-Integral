#pragma once

#define FOR_EACH(item, array, size) \
    for (unsigned __i = 0; __i < size && ((item = array[__i]), 1); __i++)

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>

#define mem_alloca(size) _alloca(size)
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

#define FORCE_INLINE __attribute__((always_inline))
#define mem_alloca(size) alloca(size)
#endif
