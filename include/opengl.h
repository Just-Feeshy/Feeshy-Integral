#pragma once

#ifdef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include <stdbool.h>

#if defined(EMSCRIPTEN)
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#define GL_TIME_ELAPSED 0x88BF

#elif defined(MACOSX)

#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#define glBindFramebuffer glBindFramebufferEXT
#define glBindRenderbuffer glBindRenderbufferEXT
#define glGenFramebuffers glGenFramebuffersEXT
#define glDeleteFramebuffers glDeleteFramebuffersEXT
#define glGenRenderbuffers glGenRenderbuffersEXT
#define glDeleteRenderbuffers glDeleteRenderbuffersEXT
#define glFramebufferRenderbuffer glFramebufferRenderbufferEXT
#define glFramebufferTexture2D glFramebufferTexture2DEXT
#define glRenderbufferStorage glRenderbufferStorageEXT
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#define glGenerateMipmap glGenerateMipmapEXT
#define glGetFramebufferAttachmentParameteriv glGetFramebufferAttachmentParameterivEXT
#define glGetRenderbufferParameteriv glGetRenderbufferParameterivEXT
#define glIsFramebuffer glIsFramebufferEXT
#define glIsRenderbuffer glIsRenderbufferEXT

#elif defined(WINDOWS)

#include <windows.h>

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

typedef enum {
    GL_SHADER_ATTR_FLOAT = 0,
    GL_SHADER_ATTR_VEC2,
    GL_SHADER_ATTR_VEC3,
    GL_SHADER_ATTR_VEC4
} GL_SHADER_ATTR_TYPE;

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

void opengl_init();
void opengl_begin(SDL_Window* window);
unsigned opengl_load_vertex_buffer(const void* buffer, size_t size);
void opengl_gen_vertex_arrays(GLsizei n, GLuint* arrays);
void opengl_set_vertex_attr(uint32_t index, uint32_t size, uint32_t type, bool normalized, uint32_t stride, uint32_t offset);
void opengl_set_vertex_attr_default(uint32_t index, const void* buffer, uint32_t attr_type, uint32_t count);
void opengl_bind_vertex_array(GLuint array);
void opengl_destroy_vertex_array(GLuint array);
void opengl_destroy_vertex_buffer(GLuint buffer);
void opengl_clear();

const uint32_t opengl_get_integerv(GLenum pname);
