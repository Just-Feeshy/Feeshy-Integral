#pragma once

#include <SDL.h>

#ifdef USE_EGL

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL_opengles2.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#endif

#else
#include <SDL_opengl.h>
#endif

typedef struct pipeline_core_gl {
    unsigned programId;
	char **textures;
	int *textureValues;
	int textureCount;
} PIPELINE_CORE;

void opengl_init(SDL_Window* window);
void opengl_begin(SDL_Window* window);
void opengl_clear();
