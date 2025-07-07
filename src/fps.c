#include "fps.h"
#include <stdio.h>
#include <string.h>

static void fps_collect_results(struct fps_counter* fps);
static void fps_calculate_average(struct fps_counter* fps);

void fps_init(struct fps_counter* fps) {
    if (!fps) return;

    memset(fps, 0, sizeof(struct fps_counter));
    glGenQueries(FPS_QUERY_COUNT, fps->queries);

    fps->current_fps = 0.0f;
    fps->avg_fps = 0.0f;
    fps->current_query = 0;
    fps->valid_queries = 0;
    fps->query_active = false;
    fps->initialized = true;
    fps->time_index = 0;
    fps->time_count = 0;
    fps->frame_count = 0;
    fps->total_gpu_time = 0;

    printf("FPS Counter initialized with GPU timing\n");
}

void fps_update_begin(struct fps_counter* fps) {
    if (!fps || !fps->initialized) return;

    if (fps->query_active) return;

    glBeginQuery(GL_TIME_ELAPSED, fps->queries[fps->current_query]);
    fps->query_active = true;
}

void fps_update_end(struct fps_counter* fps) {
    if (!fps || !fps->initialized || !fps->query_active) return;

    glEndQuery(GL_TIME_ELAPSED);
    fps->query_active = false;

    fps->current_query = (fps->current_query + 1) % FPS_QUERY_COUNT;
    if (fps->valid_queries < FPS_QUERY_COUNT) {
        fps->valid_queries++;
    }

    fps_collect_results(fps);

    fps->frame_count++;
}

static void fps_collect_results(struct fps_counter* fps) {
    if (fps->valid_queries == 0) return;

    for (int i = 0; i < fps->valid_queries; i++) {
        int query_idx = (fps->current_query - fps->valid_queries + i + FPS_QUERY_COUNT) % FPS_QUERY_COUNT;

        GLint available = 0;
        glGetQueryObjectiv(fps->queries[query_idx], GL_QUERY_RESULT_AVAILABLE, &available);

        if (available) {
            GLuint64 gpu_time_ns = 0;
            glGetQueryObjectui64v(fps->queries[query_idx], GL_QUERY_RESULT, &gpu_time_ns);

            fps->gpu_times[fps->time_index] = gpu_time_ns;
            fps->time_index = (fps->time_index + 1) % FPS_HISTORY_SIZE;
            if (fps->time_count < FPS_HISTORY_SIZE) {
                fps->time_count++;
            }

            fps->total_gpu_time += gpu_time_ns;

            if (gpu_time_ns > 0) {
                float frame_time_seconds = (float)gpu_time_ns / 1000000000.0f;
                fps->current_fps = ceilf(1.0f / frame_time_seconds);
            }

            fps_calculate_average(fps);
            fps->valid_queries--;
            break;
        }
    }
}

static void fps_calculate_average(struct fps_counter* fps) {
    if (fps->time_count == 0) return;

    uint64_t total_time = 0;
    for (int i = 0; i < fps->time_count; i++) {
        total_time += fps->gpu_times[i];
    }

    if (total_time > 0) {
        float avg_frame_time = (float)total_time / (float)fps->time_count / 1000000000.0f;
        fps->avg_fps = ceilf(1.0f / avg_frame_time);
    }
}

float fps_get_current(const struct fps_counter* fps) {
    return fps ? fps->current_fps : 0.0f;
}

float fps_get_average(const struct fps_counter* fps) {
    return fps ? fps->avg_fps : 0.0f;
}

void fps_reset(struct fps_counter* fps) {
    if (!fps) return;

    fps->current_fps = 0.0f;
    fps->avg_fps = 0.0f;
    fps->time_count = 0;
    fps->time_index = 0;
    fps->frame_count = 0;
    fps->total_gpu_time = 0;

    memset(fps->gpu_times, 0, sizeof(fps->gpu_times));
}

void fps_destroy(struct fps_counter* fps) {
    if (!fps || !fps->initialized) return;

    if (fps->query_active) {
        glEndQuery(GL_TIME_ELAPSED);
        fps->query_active = false;
    }

    glDeleteQueries(FPS_QUERY_COUNT, fps->queries);
    memset(fps, 0, sizeof(struct fps_counter));
}
