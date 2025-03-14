#pragma once

#include <SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <SDL_opengles2.h>
#elif defined(MACOSX)
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#else
#include <SDL_opengl.h>
#endif

#if defined(USE_GLES) && defined(EXT_disjoint_timer_query)
#define GL_TIME_ELAPSED GL_TIME_ELAPSED_EXT

PFNGLGENQUERIESEXTPROC glGenQueries = NULL;
PFNGLBEGINQUERYEXTPROC glBeginQuery = NULL;
PFNGLENDQUERYEXTPROC glEndQuery = NULL;
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuiv = NULL;
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
