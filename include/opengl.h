#pragma once

#ifndef EMSCRIPTEN
#include <SDL.h>
#endif

#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#define GL_TIME_ELAPSED 0x88BF
#elif defined(MACOSX)
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#else
#include <SDL_opengl.h>
#endif

#ifdef EMSCRIPTEN
#define GL_APP_PROFILE_MASK SDL_GL_CONTEXT_PROFILE_ES
#define GL_APP_MAJOR_VERSION 3
#define GL_APP_MINOR_VERSION 0
#else
#define GL_APP_PROFILE_MASK SDL_GL_CONTEXT_PROFILE_CORE
#define GL_APP_MAJOR_VERSION 4
#define GL_APP_MINOR_VERSION 1
#endif

typedef struct pipeline_core_gl {
    unsigned programId;
	char **textures;
	int *textureValues;
	int textureCount;
} PIPELINE_CORE;

typedef struct constant_location_core_gl {
    int location;
    unsigned type;
} CONSTANT_LOCATION_CORE;

void opengl_init(SDL_Window* window);
void opengl_begin(SDL_Window* window);
void opengl_gen_vertex_arrays(GLsizei n, GLuint* arrays);
void opengl_bind_vertex_array(GLuint array);
void opengl_clear();

const uint32_t opengl_get_integerv(GLenum pname);
