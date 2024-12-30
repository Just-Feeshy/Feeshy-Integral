#include <cam_matrices.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>
#include <math.h>

#define UP (vec3){0.0f, 1.0f, 0.0f}

void update_rotation(cam_matrices* cam) {
    float cos_y = cos(cam->vertical_angle);
    float sin_y = sin(cam->vertical_angle);
    float cos_x = cos(cam->horizontal_angle);
    float sin_x = sin(cam->horizontal_angle);

    cam->look_at[0] = sin_x * cos_y;
    cam->look_at[1] = sin_y;
    cam->look_at[2] = cos_x * cos_y;

    glm_vec3_normalize_to(cam->look_at, cam->front);
    glm_vec3_crossn(cam->front, UP, cam->right);
    glm_vec3_crossn(cam->right, cam->front, cam->up);
}

void update_view_matrix(cam_matrices* cam) {
    vec3 center = {
        cam->position[0] + cam->look_at[0],
        cam->position[1] + cam->look_at[1],
        cam->position[2] + cam->look_at[2]
    };

    glm_mat4_identity(cam->cam.view);
    glm_lookat(cam->position, center, (vec3){0.0f, 1.0f, 0.0f}, cam->cam.view);
}

void update_projection_matrix(cam_matrices* cam, float aspect_ratio) {
    glm_mat4_identity(cam->cam.projection);
    glm_perspective(glm_rad(45.0f), aspect_ratio, cam->cam.near, cam->cam.far, cam->cam.projection);
}

cam_matrices create_cam_matrices() {
    cam_block cam_blck = {
        .projection = GLM_MAT4_IDENTITY_INIT,
        .view = GLM_MAT4_IDENTITY_INIT,
        .position = {0.0f, 0.0f, -10.0f},
        .far = 9600.0f,
        .near = 0.05f
    };

    vec3 look_at = {0.0f, 0.0f, 0.0f};

    cam_matrices cam = {
        .cam = cam_blck,
        .look_at = {look_at[0], look_at[1], look_at[2]},
        .front = {0.0f, 0.0f, 0.0f},
        .right = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 0.0f, 0.0f},
        .horizontal_angle = atan2(look_at[0], look_at[2]),
        .vertical_angle = asin(look_at[1]),
        .position = {0.0f, 0.0f, 0.0f},
    };

    return cam;
}

void init_cam_matrices(cam_matrices* cam) {
    update_rotation(cam);
    update_view_matrix(cam);
}
