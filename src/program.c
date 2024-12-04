#include <input.h>
#include <opengl.h>
#include <program.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

typedef struct program_package {
    struct inputs* in;
    struct SDL_Window* window;
    struct SDL_GLContext* context;
    bool active;
    bool dirty_event;
} program_package;

program_package main_program;

static void create_window(const char* title, int w, int h) {
    #ifdef __EMSCRIPTEN__
    SDL_CreateWindowAndRenderer(w, h, 0, &main_program.window, 0);
    #else
    main_program.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
    #endif

    main_program.context = SDL_GL_CreateContext(main_program.window);
}

static void program_handle_event(SDL_Event* event) {
    switch(event->type) {
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

static void program_update() {
    SDL_Event event;
	event.type = -1;

    #ifndef __EMSCRIPTEN__
    if(main_program.active && (main_program.dirty_event || SDL_WaitEvent(&event))) {
        main_program.dirty_event = false;
        program_handle_event(&event);
        event.type = -1;

        if(!main_program.active) {
            return;
        }
    }
    #endif

    while(SDL_PollEvent(&event)) {
        program_handle_event(&event);
        event.type = -1;

        if(!main_program.active) {
            return;
        }
    }
}

void program_init(const char* name, int w, int h) {
    if(main_program.active) {
        return;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if(main_program.context && SDL_GL_MakeCurrent(main_program.window, main_program.context) == 0) {
        #ifdef ENABLE_VSYNC
        SDL_GL_SetSwapInterval(1);
        #else
        SDL_GL_SetSwapInterval(0);
        #endif
    }

    create_window(name, w, h);
    opengl_init(main_program.window);

    main_program.active = true;
    main_program.in = malloc(sizeof(struct inputs));
    inputs_init(main_program.in);
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

    SDL_GL_DeleteContext(main_program.context);
    SDL_DestroyWindow(main_program.window);
    SDL_Quit();

    free(main_program.in);
    main_program.active = false;
}
