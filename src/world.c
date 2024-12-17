#include <stdint.h>
#include <world.h>
#include <input.h>

static void world_input_callback_impl(uint64_t control_status) {
}

void world_init() {
    printf("World initialized\n");
    static InputCallback world_input_callback = world_input_callback_impl;
    inputs_init_callback(&world_input_callback);
}
