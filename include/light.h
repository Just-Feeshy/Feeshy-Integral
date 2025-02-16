#pragma once

typedef struct {
    float kB;
    float kT;
    float kR;
    float g;
} brdf_light_block;

void init_brdf_light_block(brdf_light_block* block);
