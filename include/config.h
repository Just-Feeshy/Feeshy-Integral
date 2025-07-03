#pragma once


// This file is suppose to be used for
// compile time configurations

#ifndef FRAGMENT_SELECTOR
#define FRAGMENT_SELECTOR 2
#endif

#ifndef GRID_SIZE
#define GRID_SIZE 128
#endif

#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 128
#endif

#ifndef MAX_ITERATIONS
#define MAX_ITERATIONS 10000
#endif

#ifndef ENABLE_NUKLEAR
#define ENABLE_NUKLEAR 1
#endif

// Control Type of FBO that gets preprocessed
#ifndef USE_FBO_WORLD
#define USE_FBO_WORLD 1
#endif

// Shader locations
// These are the locations of the attributes in the shader

#define POSITION_ATTR_LOCATION 0
#define TEXCOORD_ATTR_LOCATION 1
#define NORMAL_ATTR_LOCATION 2
#define TANGENT_ATTR_LOCATION 3
#define TEXCOORD2_ATTR_LOCATION 4
#define INDICES_ATTR_LOCATION 5
