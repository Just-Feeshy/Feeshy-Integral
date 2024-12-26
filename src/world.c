#include <stdint.h>
#include <world.h>
#include <cam_matrices.h>
#include <uniform_block_state.h>
#include <input.h>

static cam_matrices cam;
static uniform_block ubo;
static sized_shader_block** block;

static void world_input_callback_impl(uint64_t control_status) {
}

static void render_cam() {
    update_rotation(&cam);
    update_view_matrix(&cam);
    set_ssbo_data(*block, &cam.cam, sizeof(cam_block));
}

void world_init() {
    printf("World initialized\n");
    static InputCallback world_input_callback = world_input_callback_impl;
    inputs_init_callback(&world_input_callback);
    init_ubo(&ubo);

    cam = create_cam_matrices();
    init_cam_matrices(&cam);

    block = (sized_shader_block**)malloc(sizeof(sized_shader_block*));
    *block = create_ssbo(&ubo, GL_UNIFORM_BUFFER, sizeof(cam_matrices));
}

void world_aspect_ratio(float width, float height) {
    update_projection_matrix(&cam, width / height);
    set_ssbo_data(*block, &cam.cam, sizeof(cam_block));
}

void world_begin(graphics_pipeline* pipe) {
    // render_cam();
    bind_ubo_with_name(&ubo, "CamBlock", block, pipe);
}

void world_end(graphics_pipeline* pipe) {
    // unbind_ubo_just_ssbo(&ubo, block, pipe);
    unbind_ubo(&ubo, 0, *block, pipe);
}
