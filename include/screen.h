#pragma once
#include <buffers.h>
#include <stdint.h>

#ifndef EMSCRIPTEN
extern uint32_t ms_time_elapsed;
#endif

void screen_init(int w, int h);
void screen_render();
