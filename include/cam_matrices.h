#pragma once

#include <cglm/vec3.h>

typedef struct cam_block {
    mat4 projection;
    mat4 view;
    float far;
    float near;
} cam_block;

typedef struct cam_matrices {
    cam_block cam;
    vec3 look_at;
    float horizontal_angle;
    float vertical_angle;
    vec3 position;
} cam_matrices;

cam_matrices create_cam_matrices();
void init_cam_matrices(cam_matrices* cam);
