#pragma once

typedef struct {
    float kB;
    float kT;
    float kR;
    float g;
} brdf_light_block;

brdf_light_block create_brdf_light_block();
