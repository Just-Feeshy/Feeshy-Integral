#pragma once

// This file is suppose to be used for
// compile time configurations

#define GRID_SIZE 128
#define FRAGMENT_SELECTOR 2
#define DEFAULT_BUFFER_SIZE 128
#define MAX_ITERATIONS 10000

// Control Type of FBO that gets preprocessed
#define USE_FBO_WORLD 1
#define USE_FBO_CUBEMAP 0

// Shader locations
// These are the locations of the attributes in the shader

#define POSITION_ATTR_LOCATION 0
#define TEXCOORD_ATTR_LOCATION 1
#define NORMAL_ATTR_LOCATION 2
#define TANGENT_ATTR_LOCATION 3
#define TEXCOORD2_ATTR_LOCATION 4
#define INDICES_ATTR_LOCATION 5
