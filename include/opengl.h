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

#define NEED_EXT
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#include <GL/gl.h>

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

// #ifdef NEED_EXT
// #define DECLARE_EXTENSIONS
// #include <gl_ext.h>
// #undef DECLARE_EXTENSIONS
// #endif

#ifdef WINDOWS
typedef HDC WinDC;
typedef HGLRC GLCtx;
#else
typedef void *WinDC;
typedef void *GLCtx;
#endif


#ifndef GL_BUFFER_SIZE

#define GL_BUFFER_SIZE                0x8764
#define GL_BUFFER_USAGE               0x8765
#define GL_ARRAY_BUFFER               0x8892
#define GL_ELEMENT_ARRAY_BUFFER       0x8893
#define GL_ARRAY_BUFFER_BINDING       0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                  0x88B8
#define GL_WRITE_ONLY                 0x88B9
#define GL_READ_WRITE                 0x88BA
#define GL_BUFFER_ACCESS              0x88BB
#define GL_BUFFER_MAPPED              0x88BC
#define GL_BUFFER_MAP_POINTER         0x88BD
#define GL_STREAM_DRAW                0x88E0
#define GL_STREAM_READ                0x88E1
#define GL_STREAM_COPY                0x88E2
#define GL_STATIC_DRAW                0x88E4
#define GL_STATIC_READ                0x88E5
#define GL_STATIC_COPY                0x88E6
#define GL_DYNAMIC_DRAW               0x88E8
#define GL_DYNAMIC_READ               0x88E9
#define GL_DYNAMIC_COPY               0x88EA
#define GL_COMPILE_STATUS             0x8B81
#define GL_LINK_STATUS                0x8B82
#define GL_VALIDATE_STATUS            0x8B83
#define GL_INFO_LOG_LENGTH            0x8B84
#define GL_ATTACHED_SHADERS           0x8B85
#define GL_ACTIVE_UNIFORMS            0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH  0x8B87
#define GL_SHADER_SOURCE_LENGTH       0x8B88
#define GL_VERTEX_SHADER              0x8B31
#define GL_FRAGMENT_SHADER            0x8B30
#define GL_TEXTURE0                   0x84C0
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_POINT_SMOOTH
#define GL_POINT_SMOOTH 0x0B10
#endif

#ifndef GL_LINE_SMOOTH
#define GL_LINE_SMOOTH  0x0B20
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
