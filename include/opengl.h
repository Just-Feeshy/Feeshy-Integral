#pragma once

#include <SDL.h>


#ifdef EMSCRIPTEN
#define USE_EGL2
#include <emscripten.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(MACOSX)
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#else
#include <SDL_opengl.h>
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
