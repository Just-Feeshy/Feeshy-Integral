#pragma once

#include <SDL.h>

#ifdef USE_EGL

<<<<<<< Updated upstream
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL_opengles2.h>
#else
=======
#ifdef EMSCRIPTEN
#define GL_TIME_ELAPSED 0x88BF
#include <emscripten.h>

#ifdef USE_EGL2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GLES3/gl3.h>
#endif

#elif defined(MACOSX)
>>>>>>> Stashed changes
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#endif

#else
#include <SDL_opengl.h>
#endif

#ifdef GL_ES_VERSION_3_0
    #define GL_RED_FORMAT GL_RED
    #define GL_RG_FORMAT GL_RG
#else
    #define GL_RED_FORMAT GL_LUMINANCE
    #define GL_RG_FORMAT GL_LUMINANCE_ALPHA
#endif

#ifdef EXT_disjoint_timer_query
#define GL_TIME_ELAPSED GL_TIME_ELAPSED_EXT
#define GL_QUERY_RESULT GL_QUERY_RESULT_EXT
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
<<<<<<< Updated upstream
=======
void opengl_gen_vertex_arrays(GLsizei n, GLuint* arrays);
void opengl_bind_vertex_array(GLuint array);
void opengl_gen_queries(GLsizei n, GLuint* ids);
void opengl_begin_query(GLenum target, GLuint id);
void opengl_end_query(GLenum target);
void opengl_get_query_objectuiv(GLuint id, GLenum pname, GLuint* params);
>>>>>>> Stashed changes
void opengl_clear();

const uint32_t opengl_get_integerv(GLenum pname);
