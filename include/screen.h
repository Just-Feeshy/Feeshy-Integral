#pragma once

#include <pipeline.h>
#include <uniform_manager.h>
#include <buffers.h>
#include <fps.h>
#include <stdint.h>

extern struct fps_counter fps_data;
extern graphics_pipeline* pipeline;
extern uint32_t ms_time_elapsed;

void screen_init(int w, int h);
void screen_render();
