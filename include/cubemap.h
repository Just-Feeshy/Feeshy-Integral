#pragma once

#include <geometry_pass.h>
#include <texture.h>

void cubemap_init(geometry_pass* g_pass);
void cubemap_create_mesh(texture* tex);
void cubemap_mesh_render();
void cubemap_render();
