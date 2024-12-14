#include <input.h>
#include <opengl.h>
#include <program.h>
#include <more_math.h>
#include <SDL_render.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define TIME_RESOLUTION UINT64_C(1000000000)
#define NAP_MULT 1
#define NAP_DIV 3

const double frame_period = 1000.0f / 60.0f;

typedef struct program_package {
    struct inputs* in;
    struct SDL_Window* window;
    struct SDL_GLContext* context;
    bool active;
    bool dirty_event;
} program_package;

typedef struct update_package {
    int currentUpdate;
    int lastUpdate;
    int nextUpdate;
    bool timerActive;
    SDL_TimerID timer;
} update_package;

program_package main_program;
update_package main_update;

static uint32_t onTimer(uint32_t interval, void* nul) {
    SDL_Event event;
    SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = 0;
	userevent.data2 = 0;
	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return 0;
}

static void create_window(const char* title, int w, int h) {
    #ifdef __EMSCRIPTEN__
    SDL_CreateWindowAndRenderer(w, h, 0, &main_program.window, 0);
    #else
    main_program.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    #endif

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
    //opengl_begin(main_program.window);
    opengl_clear();
    program_context_flip();
}

static void program_draw_update() {
    main_update.currentUpdate = SDL_GetTicks();
    main_update.lastUpdate = main_update.currentUpdate;
    main_update.nextUpdate = frame_period;


    while(main_update.nextUpdate <= main_update.currentUpdate) {
        main_update.nextUpdate += frame_period;
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
    }
}

/*
static void program_update() {
    SDL_Event event;
	event.type = -1;

    #ifndef __EMSCRIPTEN__
    if(main_program.active && (main_program.dirty_event || SDL_WaitEvent(&event))) {
        main_program.dirty_event = false;
        //program_handle_event(&event);
        event.type = -1;

        if(!main_program.active) {
            return;
        }
    }
    #endif

    while(SDL_PollEvent(&event)) {
        printf("Event type: %d\n", event.type);
        program_handle_event(&event);
        event.type = -1;

        if(!main_program.active) {
            return;
        }
    }

    main_update.currentUpdate = SDL_GetTicks();

    #ifdef __EMSCRIPTEN__
    if(main_update.currentUpdate >= main_update.nextUpdate) {
        event.type = SDL_USEREVENT;
        program_handle_event(&event);
        event.type = -1;
    }
    #else
    if(main_update.currentUpdate >= main_update.nextUpdate) {
        if(main_update.timerActive) {
            SDL_RemoveTimer(main_update.timer);
            main_update.timerActive = false;
        }

        onTimer(0, 0);
    }else if(!main_update.timerActive) {
        main_update.timer = SDL_AddTimer(main_update.nextUpdate - main_update.currentUpdate, onTimer, 0);
        main_update.timerActive = true;
    }
    #endif
}
*/

static int64_t program_get_time() {
    return SDL_GetTicks() * TIME_RESOLUTION;
}

static void program_update() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        program_handle_event(&event);
    }

    main_update.currentUpdate = SDL_GetTicks();

    if (main_update.currentUpdate >= main_update.nextUpdate) {
        main_update.nextUpdate += frame_period;
        program_update_opengl();
    }else {
        int64_t remaining_time = (int64_t)main_update.nextUpdate - program_get_time();

        while(remaining_time > frame_period / NAP_DIV) {
            uint64_t nap_time = max(0, remaining_time);
            uint32_t nap_sdl = (NAP_MULT * nap_time * 1000) / (TIME_RESOLUTION * 1000ULL * NAP_DIV);
            nap_sdl = max(nap_sdl, 1);
            SDL_Delay(nap_sdl);

            remaining_time = (int64_t)main_update.nextUpdate - program_get_time();
        }
    }
}

void program_init(const char* name, int w, int h) {
    if(main_program.active) {
        return;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return;
	}

    SDL_Init(SDL_INIT_VIDEO);

    #ifdef WINDOWS
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    #endif

    #if !defined(__EMSCRIPTEN__)
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

    create_window(name, w, h);
    opengl_init(main_program.window);

    if(main_program.context && SDL_GL_MakeCurrent(main_program.window, main_program.context) == 0) {
        #ifdef ENABLE_VSYNC
        SDL_GL_SetSwapInterval(1);
        #else
        SDL_GL_SetSwapInterval(0);
        #endif
    }

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    main_program.active = true;
    main_program.in = malloc(sizeof(struct inputs));
    inputs_init(main_program.in);

    main_update.lastUpdate = SDL_GetTicks();
    main_update.nextUpdate = main_update.lastUpdate;
    main_update.currentUpdate = SDL_GetTicks();
    main_update.timerActive = false;
}

void program_loop() {
    #ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop ();
	emscripten_set_main_loop (program_update, 0, 0);
	emscripten_set_main_loop_timing (EM_TIMING_RAF, 1);
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

    SDL_DestroyWindow(main_program.window);
    SDL_Quit();

    free(main_program.in);
    main_program.active = false;
}
