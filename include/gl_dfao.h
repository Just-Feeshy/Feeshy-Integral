#pragma once

#include <stdint.h>
#include <stdbool.h>

bool init_dfao(uint32_t* fbo);
void generate_dfao_scene(uint32_t* fbo, uint32_t source_tex_id);
void destroy_dfao(uint32_t* fbo);
