#include <stdint.h>
#include <world.h>
#include <cam_matrices.h>
#include <input.h>

static cam_matrices cam;

static void world_input_callback_impl(uint64_t control_status) {
}

void world_init() {
    printf("World initialized\n");
    static InputCallback world_input_callback = world_input_callback_impl;
    inputs_init_callback(&world_input_callback);

    cam = create_cam_matrices();
    init_cam_matrices(&cam);
}
