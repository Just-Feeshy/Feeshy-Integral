#pragma once

#ifdef USE_EGL

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#else
#include <SDL_opengl.h>
#endif
