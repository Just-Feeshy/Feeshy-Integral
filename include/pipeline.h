#pragma once

#include <opengl.h>
#include <shader.h>

typedef struct graphics_pipeline {
    PIPELINE_CORE pipeline_core;
} graphics_pipeline;

void pipeline_init(graphics_pipeline* pipeline);
void pipeline_destroy(graphics_pipeline* pipeline);
void pipeline_compile(uint32_t num_shdrs, graphics_pipeline* pipeline, shader* shaders[num_shdrs]);
void pipeline_set(graphics_pipeline* pipeline);
