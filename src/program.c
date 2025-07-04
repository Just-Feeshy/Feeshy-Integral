#define NK_SDL_GL3_IMPLEMENTATION 1

#include <program.h>
#include <screen.h>
#include <screenshot.h>
#include <world.h>
#include <more_math.h>
#include <nuklear_sdl_gl.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#define TIME_RESOLUTION UINT64_C(1000000000)
#define NAP_MULT 1
#define NAP_DIV 3

const double frame_period = 1000.0f / 60.0f;

static struct nk_context* ctx;
static struct nk_colorf bg;

static char FOV_TXT[9] = "FOV (45)";
static char buffer_url[65536] = {0}; // I really don't care of any buffer overflow here, it's just a simulation
static int length = 0;

typedef struct program_package {
    struct inputs* in;
    struct SDL_Window* window;
    struct GL_Context* context;
    bool active;
    bool dirty_event;
} program_package;

typedef struct update_package {
    int currentUpdate;
    int64_t lastUpdate;
    int nextUpdate;
    bool timerActive;
    SDL_TimerID timer;
} update_package;

program_package main_program;
update_package main_update;

static void create_window(const char* title, int w, int h) {
    main_program.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    main_program.context = SDL_GL_CreateContext(main_program.window);

    if (main_program.window == NULL) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(!main_program.context) {
        printf("Failed to create context: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

static void program_context_flip() {
    if(main_program.context) {
        SDL_GL_SwapWindow(main_program.window);
    }
}

static void program_update_opengl() {
    world_begin(pipeline);

    opengl_begin(main_program.window);
    opengl_clear();

    screen_render();
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    world_end(pipeline);

    program_context_flip();
}

static void program_draw_update() {
    main_update.currentUpdate = SDL_GetTicks();
    main_update.lastUpdate = main_update.currentUpdate;
    main_update.nextUpdate = frame_period;

    while(main_update.nextUpdate <= main_update.currentUpdate) {
        main_update.nextUpdate += frame_period;
        inputs_update(main_program.in);
    }

    program_update_opengl();
}

static void program_handle_event(SDL_Event* event) {
    switch(event->type) {
        case SDL_USEREVENT:
            program_draw_update();
            break;

        case SDL_WINDOWEVENT: // TODO: Implement this
		    break;

        case SDL_QUIT:
            main_program.active = false;
            break;

        case SDL_KEYDOWN:
            inputs_key_down(main_program.in, event->key.keysym.sym);
            break;

        case SDL_KEYUP:
            inputs_key_up(main_program.in, event->key.keysym.sym);
            break;

        case SDL_MOUSEMOTION:
            inputs_motion(main_program.in->control_status & ESCAPE, event->motion.x, event->motion.y, event->motion.xrel, event->motion.yrel);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if(nk_window_is_any_hovered(ctx)) {
                break;
            }

            main_program.in->control_status &= ~ESCAPE;
            SDL_SetRelativeMouseMode(SDL_TRUE);
            break;
    }
}

static int64_t program_get_time() {
    return SDL_GetTicks() * TIME_RESOLUTION;
}

static void program_update() {
    SDL_Event event;

    nk_input_begin(ctx);
    while (SDL_PollEvent(&event)) {
        program_handle_event(&event);
        nk_sdl_handle_event(&event);
    }
    nk_sdl_handle_grab();
    nk_input_end(ctx);

#if ENABLE_NUKLEAR == 1
    if (nk_begin(ctx, "Settings Menu", nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        static float fov = 45.0f;

        nk_layout_row_static(ctx, 20, 80, 1);
        nk_label(ctx, FOV_TXT, NK_TEXT_LEFT);

        nk_layout_row_static(ctx, 20, 100, 2);
        if(nk_slider_float(ctx, 10, &fov, 90, 1)) {
            FOV_TXT[5] = (int)fov / 10 + '0';
            FOV_TXT[6] = (int)fov % 10 + '0';
            world_update_fov(fov);
        }

        if(nk_button_label(ctx, "Reset Camera")) {
            world_reset_camera();
        }

        nk_layout_row_static(ctx, 20, 200, 1);

        {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "Elapsed Shader Time: %u", ms_time_elapsed / 1000000);
            nk_label(ctx, buffer, NK_TEXT_LEFT);
        }

        #ifdef HAS_GEOMETRY_PASS
        if(nk_button_label(ctx, "Turn On Wireframe")) {
            world_toggle_wireframe();
        }
        #endif
    }
    nk_end(ctx);
#endif

    int64_t current_time = program_get_time();
    main_update.nextUpdate = frame_period + main_update.lastUpdate;

    while ((int64_t)main_update.nextUpdate - current_time > frame_period / NAP_DIV) {
        uint64_t nap_time = max(0, (int64_t)main_update.nextUpdate - current_time);
        uint32_t nap_sdl = (NAP_MULT * nap_time * 1000) / (TIME_RESOLUTION * 1000ULL * NAP_DIV);
        nap_sdl = max(nap_sdl, 1);
        SDL_Delay(nap_sdl);

        current_time = program_get_time();
    }

    if (current_time >= main_update.nextUpdate) {
        main_update.nextUpdate += frame_period;
    }

    program_update_opengl();
    inputs_update(main_program.in);
}

void program_init(const char* name, int w, int h) {
    if(main_program.active) {
        return;
    }

    #ifdef NEED_EXT
    opengl_init_ext();
    #endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return;
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GL_APP_PROFILE_MASK);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_APP_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_APP_MINOR_VERSION);

    #ifndef EMSCRIPTEN
    SDL_SetHint (SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "0");
    SDL_SetHint (SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_SetHint (SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint (SDL_HINT_TOUCH_MOUSE_EVENTS, "1");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    #endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetRelativeMouseMode(SDL_FALSE);

    create_window(name, w, h);
    opengl_init();
    ctx = nk_sdl_init(main_program.window);

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    SDL_WarpMouseInWindow(main_program.window, w >> 1, h >> 1);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    #ifndef EMSCRIPTEN
    if(main_program.context && SDL_GL_MakeCurrent(main_program.window, main_program.context) == 0) {
        #ifdef ENABLE_VSYNC
        SDL_GL_SetSwapInterval(1);
        #else
        SDL_GL_SetSwapInterval(0);
        #endif
    }
    #endif

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    main_program.active = true;
    main_program.in = malloc(sizeof(struct inputs));
    inputs_init(main_program.in);

    main_update.lastUpdate = SDL_GetTicks();
    main_update.nextUpdate = main_update.lastUpdate;
    main_update.currentUpdate = SDL_GetTicks();
    main_update.timerActive = false;

    uniform_manager_init();
    screen_init(w, h);

    screenshot_init();
    world_init(w, h);
    world_aspect_ratio(w, h);
}

void program_set_as_escaped() {
    main_program.in->control_status |= ESCAPE;
}

void program_loop() {
    printf("Starting program loop\n");

    #ifdef EMSCRIPTEN
    emscripten_cancel_main_loop();
	emscripten_set_main_loop(program_update, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 0);
    #else
    while(main_program.active) {
        program_update();
    }
    #endif
}

void program_destroy() {
    if(!main_program.active) {
        return;
    }

    if(main_program.context) {
        SDL_GL_DeleteContext(main_program.context);
        main_program.context = NULL;
    }

    world_destroy();
    pipeline_destroy(pipeline);
    free(pipeline);

    SDL_DestroyWindow(main_program.window);
    SDL_Quit();

    free(main_program.in);
    main_program.active = false;
}

float program_get_pixel_density() {
    int window_w, window_h, pixel_w, pixel_h;
    float pixel_density = 1.0f;

    SDL_GetWindowSize(main_program.window, &window_w, &window_h);
    SDL_GetWindowSizeInPixels(main_program.window, &pixel_w, &pixel_h);
    pixel_density = (float)pixel_w / window_w;
    return pixel_density;
}

// TODO: Implement this
float program_get_elapsed_time() {
    return 1.0;
}
