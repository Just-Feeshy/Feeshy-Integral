#pragma once

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
