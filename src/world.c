#include <stdint.h>
#include <world.h>
#include <light.h>
#include <menu.h>
#include <cam_matrices.h>
#include <cglm/vec3.h>
#include <uniform_block_state.h>
#include <screenshot.h>
#include <program.h>
#include <input.h>

#ifndef EMSCRIPTEN
#define SPEED 0.1
#else
#define SPEED 0.2
#endif

#define TAU 6.28318530718

static const float LIMIT = TAU / 4.01;

// Camera

static cam_matrices cam;
static uniform_block ubo;
static sized_shader_block*** block; // This is so ugly, but it works and I'm tired, got only 2 hours of sleep last night
static float aspect_ratio;

static void render_cam() {
    update_rotation(&cam);
    update_view_matrix(&cam);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
}

static void world_input_callback_impl(uint64_t control_status, input_status input) {
    if(control_status & FORWARD) {
        vec3 front_scaled;
        glm_vec3_scale(cam.front, SPEED, front_scaled);
        glm_vec3_add(cam.cam.position, front_scaled, cam.cam.position);
    }

    if(control_status & BACKWARD) {
        vec3 backward_scaled;
        glm_vec3_scale(cam.front, SPEED, backward_scaled);
        glm_vec3_sub(cam.cam.position, backward_scaled, cam.cam.position);
    }

    if(control_status & RIGHT) {
        vec3 right_scaled;
        glm_vec3_scale(cam.right, SPEED, right_scaled);
        glm_vec3_add(cam.cam.position, right_scaled, cam.cam.position);
    }

    if(control_status & LEFT) {
        vec3 left_scaled;
        glm_vec3_scale(cam.right, SPEED, left_scaled);
        glm_vec3_sub(cam.cam.position, left_scaled, cam.cam.position);
    }

    if(control_status & UP) {
        vec3 up_scaled;
        glm_vec3_scale(cam.up, SPEED, up_scaled);
        glm_vec3_add(cam.cam.position, up_scaled, cam.cam.position);
    }

    if(control_status & DOWN) {
        vec3 down_scaled;
        glm_vec3_scale(cam.up, SPEED, down_scaled);
        glm_vec3_sub(cam.cam.position, down_scaled, cam.cam.position);
    }

    #ifndef EMSCRIPTEN
    if(control_status & SCREENSHOT && input == PRESS_DOWN) {
        printf("Screenshot Saved!\n");
        capture_screenshot();
    }
    #endif

    render_cam();
}

static void world_direction_callback_impl(int x, int y, int dx, int dy) {

    cam.horizontal_angle = fmod(cam.horizontal_angle - dx / 512.0, TAU);
    cam.vertical_angle = fclamp(cam.vertical_angle - dy / 512.0, -LIMIT, LIMIT);
    render_cam();
}

void world_init() {
    printf("World Initialized\n");
    static InputCallback world_input_callback = world_input_callback_impl;
    static InputDirectionCallback world_direction_callback = world_direction_callback_impl;
    inputs_init_callback(&world_input_callback, &world_direction_callback);
    init_ubo(&ubo);

    cam = create_cam_matrices();
    init_cam_matrices(&cam);

    block = (sized_shader_block***)malloc(sizeof(sized_shader_block**) * 2);
    *block = (sized_shader_block**)malloc(sizeof(sized_shader_block*));
    **block = create_ssbo(&ubo, GL_UNIFORM_BUFFER, sizeof(cam_matrices));

    *(block + 1) = (sized_shader_block**)malloc(sizeof(sized_shader_block*));
    render_cam();
}

void world_aspect_ratio(float width, float height) {
    aspect_ratio = width / height;

    update_projection_matrix(&cam, aspect_ratio, 45.0f);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
    glm_vec3_copy(cam.init_position, cam.cam.position);
}

void world_begin(graphics_pipeline** pipe , size_t pipe_count) {
    bind_ubo_with_name(&ubo, "CamBlock", *block, pipe, pipe_count);
}

void world_end(graphics_pipeline** pipe, size_t pipe_count) {
    // unbind_ubo_just_ssbo(&ubo, block, pipe);
    unbind_ubo(&ubo, 0, **block, pipe, pipe_count);
}


// Event Related

void world_reset_camera() {
    cam.horizontal_angle = 0.0f;
    cam.vertical_angle = 0.0f;

    glm_vec3_copy(cam.init_position, cam.cam.position);
    render_cam();
}

void world_update_fov(float fov) {
    update_projection_matrix(&cam, aspect_ratio, fov);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
}
