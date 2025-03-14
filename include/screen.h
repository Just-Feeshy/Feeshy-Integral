#pragma once
#include <buffers.h>
#include <stdint.h>

#ifdef EMSCRIPTEN
extern uint32_t ms_time_elapsed;
#else
extern uint64_t ms_time_elapsed;
#endif

void screen_init(int w, int h);
void screen_render();
