#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <opengl.h>

#define FPS_QUERY_COUNT 4
#define FPS_HISTORY_SIZE 480

struct fps_counter {
    GLuint queries[FPS_QUERY_COUNT];
    int current_query;
    int valid_queries;

    uint64_t gpu_times[FPS_HISTORY_SIZE];
    int time_index;
    int time_count;

    float current_fps;
    float avg_fps;

    bool query_active;
    bool initialized;

    uint64_t frame_count;
    uint64_t total_gpu_time;
};

void fps_init(struct fps_counter* fps);
void fps_update_begin(struct fps_counter* fps);
void fps_update_end(struct fps_counter* fps);
float fps_get_current(const struct fps_counter* fps);
float fps_get_average(const struct fps_counter* fps);
void fps_reset(struct fps_counter* fps);
void fps_destroy(struct fps_counter* fps);
