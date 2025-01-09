#pragma once

#include <cglm/vec3.h>

typedef struct {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

typedef struct cam_matrices {
    cam_block cam;
    vec3 look_at;
    vec3 front;
    vec3 right;
    vec3 up;
    float horizontal_angle;
    float vertical_angle;
    vec3 position;
} cam_matrices;

cam_matrices create_cam_matrices();
void init_cam_matrices(cam_matrices* cam);
void update_rotation(cam_matrices* cam);
void update_view_matrix(cam_matrices* cam);
void update_projection_matrix(cam_matrices* cam, float aspect_ratio, float fov);
