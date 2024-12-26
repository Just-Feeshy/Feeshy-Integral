#pragma once

#include <stdint.h>
#include <utils.h>

#define min(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	_temp_a > _temp_b ? _temp_b : _temp_a; \
})

#define max(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	_temp_a > _temp_b ? _temp_a : _temp_b; \
})

static uint64_t ceil_log2(uint32_t n) {
    return 32 - __builtin_clz(n);
}
