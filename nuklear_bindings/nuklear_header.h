#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#ifdef USE_GLES

#ifdef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL_opengles2.h>
#endif

#else
#include <SDL_opengl.h>
#endif

#include <nuklear.h>
