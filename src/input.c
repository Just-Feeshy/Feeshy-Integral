#include <input.h>

static InputCallback* input_callback;

void inputs_init(inputs* in) {
    in->mapped_inputs[INPUT_W - INPUT_A] = FORWARD;
    in->mapped_inputs[INPUT_S - INPUT_A] = BACKWARD;
    in->mapped_inputs[INPUT_D - INPUT_A] = RIGHT;
    in->mapped_inputs[INPUT_A - INPUT_A] = LEFT;
    in->mapped_inputs[INPUT_Q - INPUT_A] = UP;
    in->mapped_inputs[INPUT_E - INPUT_A] = DOWN;

    in->control_status = 0;
}

void inputs_init_callback(InputCallback* callback) {
    input_callback = callback;
    printf("Input callback initialized\n");
}

void inputs_key_down(inputs* in, SDL_Keycode key) {
    if(sizeof(in->mapped_inputs) / sizeof(in->mapped_inputs[0]) < key - INPUT_A) {
        return;
    }

    if(in->mapped_inputs[key - INPUT_A] == 0) {
        return;
    }

    in->control_status |= in->mapped_inputs[key - INPUT_A];
    (*input_callback)(in->control_status);
}

void inputs_key_up(inputs* in, SDL_Keycode key) {
    if(sizeof(in->mapped_inputs) / sizeof(in->mapped_inputs[0]) < key - INPUT_A) {
        return;
    }

    if(in->mapped_inputs[key - SDLK_a] == 0) {
        return;
    }

    in->control_status &= ~in->mapped_inputs[key - INPUT_A];
    (*input_callback)(in->control_status);
}

void inputs_update(inputs* in) {
    if(in->control_status == 0) {
        return;
    }

    (*input_callback)(in->control_status);
}
