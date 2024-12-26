#pragma once

#include <pipeline.h>

void world_init();
void world_update(uint64_t control_status);
void world_begin(graphics_pipeline* pipe);
void world_end(graphics_pipeline* pipe);
void world_aspect_ratio(float width, float height);
