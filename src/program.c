#include <program.h>
#include <SDL.h>
#include <opengl.h>

SDL_Window* window;

static void create_window(const char* title, int w, int h) {
    #ifdef __EMSCRIPTEN__
    SDL_CreateWindowAndRenderer(w, h, 0, &window, 0);
    #else
    SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
    #endif
}

void program_init(const char* name, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    create_window(name, w, h);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}
