#pragma once

#include <stdint.h>

void gpu_timer_query_init();
void gpu_timer_query_begin();
void gpu_timer_query_end();
uint32_t gpu_timer_query_result();
