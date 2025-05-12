#pragma once

#include <opengl.h>
#include <pipeline.h>
#include <cglm/mat4.h>

typedef struct constant_location {
    CONSTANT_LOCATION_CORE core;
    const char* name;
} constant_location;

void uniform_manager_init();
void create_constant_location(graphics_pipeline* pipe, const char* name);
void set_uniform_int(const char* name, int value);
void set_uniform_float(const char* name, float value);
void set_uniform_vec2(const char* name, float x, float y);
void set_uniform_mat4(const char* name, mat4 mat);
int get_uniform_block(int programIndex, const char* name);
void set_uniform_block(const char* name, int binding, graphics_pipeline* pipe);
