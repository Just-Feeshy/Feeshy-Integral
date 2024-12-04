#include <program.h>
#include <stdio.h>
#include <SDL.h>

void program_init(const char* name, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    printf("Initializing program %s with dimensions %dx%d\n", name, w, h);
}
