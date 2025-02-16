#pragma once
#include <buffers.h>
#include <stdint.h>

extern uint64_t ms_time_elapsed;

void screen_init(int w, int h);
void screen_render();
