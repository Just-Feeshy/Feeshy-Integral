#include <fps.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef EMSCIPTEN

#include <emscripten.h>
#include <emscripten/html5.h>

#endif

static bool timer_queries_supported = false;
static bool timer_queries_checked = false;

static void fps_collect_results(struct fps_counter* fps);
static void fps_calculate_average(struct fps_counter* fps);
static bool fps_check_timer_query_support(void);

#ifdef EMSCIPTEN

EM_JS(double, get_performance_now, (), {
    return performance.now();
});

#endif

static bool fps_check_timer_query_support(void) {
    if (timer_queries_checked) {
        return timer_queries_supported;
    }

    timer_queries_checked = true;

#ifdef EMSCIPTEN
    // Check for WebGL2 timer query extension
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (extensions && strstr(extensions, "EXT_disjoint_timer_query_webgl2")) {
        timer_queries_supported = true;
        printf("WebGL2 timer queries supported\n");
    } else {
        timer_queries_supported = false;
        printf("WebGL2 timer queries not supported, using fallback timing\n");
    }
#else
    // Desktop OpenGL - assume timer queries are supported
    timer_queries_supported = true;
#endif

    return timer_queries_supported;
}

void fps_init(struct fps_counter* fps) {
    if (!fps) return;
    memset(fps, 0, sizeof(struct fps_counter));

    fps->current_fps = 0.0f;
    fps->avg_fps = 0.0f;
    fps->current_query = 0;
    fps->valid_queries = 0;
    fps->query_active = false;
    fps->time_index = 0;
    fps->time_count = 0;
    fps->frame_count = 0;
    fps->total_gpu_time = 0;

    if (fps_check_timer_query_support()) {
        printf("OpenGL context: %p\n", SDL_GL_GetCurrentContext());
        printf("GL_TIME_ELAPSED constant: 0x%x\n", GL_TIME_ELAPSED);

        printf("glGenQueries function pointer: %p\n", (void*)glGenQueries);
        printf("glBeginQuery function pointer: %p\n", (void*)glBeginQuery);
        printf("glEndQuery function pointer: %p\n", (void*)glEndQuery);

        if (!&glGenQueries || !&glBeginQuery || !&glEndQuery) {
            printf("Timer query function pointers are NULL - not supported\n");
            timer_queries_supported = false;
        } else {
            while (glGetError() != GL_NO_ERROR);

            glFinish();

            for (int i = 0; i < FPS_QUERY_COUNT; i++) {
                glGenQueries(1, &fps->queries[i]);
                GLenum error = glGetError();
                if (error != GL_NO_ERROR) {
                    printf("glGenQueries failed on query %d with error 0x%x\n", i, error);
                    timer_queries_supported = false;
                    break;
                } else {
                    printf("Generated query %d: ID=%u\n", i, fps->queries[i]);
                }
            }

            if (timer_queries_supported) {
                printf("Successfully generated all %d timer queries\n", FPS_QUERY_COUNT);
                printf("FPS Counter initialized with GPU timing\n");
            }
        }
    }


    fps->initialized = true;
}

void fps_update_begin(struct fps_counter* fps) {
    if (!fps || !fps->initialized) return;

    if (timer_queries_supported) {
        if (fps->query_active) return;
        glBeginQuery(GL_TIME_ELAPSED, fps->queries[fps->current_query]);
        fps->query_active = true;
    } else {
#ifdef EMSCIPTEN
        fps->frame_start_time = emscripten_performance_now();
#endif
    }
}

void fps_update_end(struct fps_counter* fps) {
    if (!fps || !fps->initialized) return;

    if (timer_queries_supported) {
        if (!fps->query_active) return;
        glEndQuery(GL_TIME_ELAPSED);
        fps->query_active = false;
        fps->current_query = (fps->current_query + 1) % FPS_QUERY_COUNT;
        if (fps->valid_queries < FPS_QUERY_COUNT) {
            fps->valid_queries++;
        }
        fps_collect_results(fps);
    }

    fps->frame_count++;
}


static void fps_collect_results(struct fps_counter* fps) {
    if (fps->valid_queries == 0) return;

    for (int i = 0; i < fps->valid_queries; i++) {
        int query_idx = (fps->current_query - fps->valid_queries + i + FPS_QUERY_COUNT) % FPS_QUERY_COUNT;
        GLuint available = 0;
        glGetQueryObjectuiv(fps->queries[query_idx], GL_QUERY_RESULT_AVAILABLE, &available);

        if (available) {
            GLuint gpu_time_ns = 0;
            glGetQueryObjectuiv(fps->queries[query_idx], GL_QUERY_RESULT, &gpu_time_ns);

            fps->gpu_times[fps->time_index] = gpu_time_ns;
            fps->time_index = (fps->time_index + 1) % FPS_HISTORY_SIZE;
            if (fps->time_count < FPS_HISTORY_SIZE) {
                fps->time_count++;
            }
            fps->total_gpu_time += gpu_time_ns;

            if (gpu_time_ns > 0) {
                float frame_time_seconds = (float)gpu_time_ns / 1000000000.0f;
                fps->current_fps = 1.0f / frame_time_seconds;
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
        fps->avg_fps = 1.0f / avg_frame_time;
    }
}

float fps_get_current(const struct fps_counter* fps) {
    return fps ? fps->current_fps : 0.0f;
}

float fps_get_average(const struct fps_counter* fps) {
    return fps ? fps->avg_fps : 0.0f;
}

bool fps_using_gpu_timing(const struct fps_counter* fps) {
    return fps && fps->initialized && timer_queries_supported;
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

#ifdef EMSCRIPTEN
    if (!timer_queries_supported) {
        fps->last_frame_time = 0;
    }
#endif
}

void fps_destroy(struct fps_counter* fps) {
    if (!fps || !fps->initialized) return;

    if (timer_queries_supported) {
        if (fps->query_active) {
            glEndQuery(GL_TIME_ELAPSED);
            fps->query_active = false;
        }
        glDeleteQueries(FPS_QUERY_COUNT, fps->queries);
    }

    memset(fps, 0, sizeof(struct fps_counter));
}
