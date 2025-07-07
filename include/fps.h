#pragma once

#include <stdint.h>

#define FRAME_CAP 120

struct fps_counter {
    int frame_count;
    uint32_t last_fps_time;
    float current_fps;
    float avg_fps;
    float min_fps;
    float max_fps;
    float frame_times[FRAME_CAP];  // Store last 60 frame times for smoothing
    int frame_time_index;
    uint32_t last_frame_time;
};

void fps_init(struct fps_counter* fps);
void fps_update(struct fps_counter* fps);
