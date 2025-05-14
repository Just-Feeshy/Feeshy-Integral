#pragma once
#include <uniform_manager.h>
#include <buffers.h>
#include <stdint.h>

extern uint32_t ms_time_elapsed;

void screen_init(int w, int h);
void screen_render();
