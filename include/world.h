#pragma once

#include <pipeline.h>

void world_init(int w, int h);
void world_update(uint64_t control_status);
void world_begin(graphics_pipeline* pipe);
void world_end(graphics_pipeline* pipe);
void world_aspect_ratio(float width, float height);
void world_setup_uniforms();
void world_toggle_wireframe();
void world_destroy();
