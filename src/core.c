#include <core.h>
#include <string.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#ifndef MAX_FILEPATH_LENGTH
    #if defined(_WIN32)
        #define MAX_FILEPATH_LENGTH 256
    #else
        #define MAX_FILEPATH_LENGTH 4096
    #endif
#endif

const char* get_directory_path(const char* path) {
    static char directory_path[MAX_FILEPATH_LENGTH] = {0};
    SDL_strlcpy(directory_path, path, sizeof(directory_path));

    char* last_slash = SDL_strrchr(directory_path, '/');
    if(!last_slash) {
        last_slash = SDL_strrchr(directory_path, '\\');
    }

    if(last_slash) {
        *(last_slash + 1) = '\0';
    } else {
        SDL_strlcpy(directory_path, "./", sizeof(directory_path));
    }

    return directory_path;
}
