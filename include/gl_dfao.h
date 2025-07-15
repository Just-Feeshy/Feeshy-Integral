#pragma once

#include <utils.h>
#include <geometry_pass.h>

#if defined(USE_DFAO) && defined(HAS_GEOMETRY_PASS)
extern texture* mesh_volume_texture;
#endif

void dfao_test_world(
#ifdef HAS_GEOMETRY_PASS
    geometry_pass* g_pass
#endif
);

void dfao_test_world_render();
