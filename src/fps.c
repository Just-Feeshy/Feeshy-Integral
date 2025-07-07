#include <fps.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_timer.h>
#else
#include <SDL_timer.h>
#endif

void fps_init(struct fps_counter* fps) {
    fps->frame_count = 0;
    fps->last_fps_time = SDL_GetTicks();
    fps->current_fps = 0.0f;
    fps->avg_fps = 0.0f;
    fps->min_fps = 999.0f; // Start with a high value
    fps->max_fps = 0.0f; // Start with a low value
    fps->frame_time_index = 0;
    fps->last_frame_time = SDL_GetTicks();

    for(int i = 0; i < FRAME_CAP; i++) {
        fps->frame_times[i] = 0.0f; // Initialize frame times to 0
    }
}

void fps_update(struct fps_counter* fps) {
    uint32_t current_time = SDL_GetTicks();
    fps->frame_count++;

    float frame_time = (float)(current_time - fps->last_frame_time);
    fps->last_frame_time = current_time;

    fps->frame_times[fps->frame_time_index] = frame_time;
    fps->frame_time_index = (fps->frame_time_index + 1) % FRAME_CAP;

    if(current_time - fps->last_fps_time >= 1000) {
        fps->current_fps = (float)fps->frame_count / ((current_time - fps->last_fps_time) / 1000.0f);

        if(fps->current_fps < fps->min_fps) {
            fps->min_fps = fps->current_fps;
        }

        if(fps->current_fps > fps->max_fps) {
            fps->max_fps = fps->current_fps;
        }

        float total_time = 0.0f;
        for(int i = 0; i < FRAME_CAP; i++) {
            total_time += fps->frame_times[i];
        }

        float avg_time = total_time / FRAME_CAP;
        fps->avg_fps = avg_time > 0 ? 1000.0f / avg_time : 0.0f;

        fps->frame_count = 0;
        fps->last_fps_time = current_time;
    }
}
