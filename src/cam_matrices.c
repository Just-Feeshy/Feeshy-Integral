#include <cam_matrices.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>
#include <math.h>

#define COS_X 0.0
#define SIN_X 1.0

static void update_rotation(cam_matrices* cam) {
    float cos_y = cos(cam->vertical_angle);
    float sin_y = sin(cam->vertical_angle);

    cam->look_at[0] = SIN_X * cos_y;
    cam->look_at[1] = sin_y;
    cam->look_at[2] = COS_X * cos_y;
}

static void update_view_matrix(cam_matrices* cam) {
    vec3 center = {
        cam->position[0] + cam->look_at[0],
        cam->position[1] + cam->look_at[1],
        cam->position[2] + cam->look_at[2]
    };

    glm_mat4_identity(cam->cam.view);
    glm_lookat(cam->position, center, (vec3){0.0f, 1.0f, 0.0f}, cam->cam.view);
}

cam_matrices create_cam_matrices() {
    cam_block cam_blck = {
        .projection = GLM_MAT4_IDENTITY_INIT,
        .view = GLM_MAT4_IDENTITY_INIT,
        .far = 960.0f,
        .near = 0.05f
    };

    vec3 look_at = {0.0f, 0.0f, 0.0f};

    cam_matrices cam = {
        .cam = cam_blck,
        .look_at = {0.0f, 0.0f, 0.0f},
        .horizontal_angle = atan2(look_at[0], look_at[2]),
        .vertical_angle = asin(look_at[1]),
        .position = {0.0f, 0.0f, 0.0f}
    };

    return cam;
}

void init_cam_matrices(cam_matrices* cam) {
    update_rotation(cam);
    update_view_matrix(cam);
}
