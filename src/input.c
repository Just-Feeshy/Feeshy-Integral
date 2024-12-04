#include <input.h>

void inputs_init(inputs* in) {
    in->mapped_inputs[INPUT_W - INPUT_A] = FORWARD;
    in->mapped_inputs[INPUT_S - INPUT_A] = BACKWARD;
    in->mapped_inputs[INPUT_D - INPUT_A] = RIGHT;
    in->mapped_inputs[INPUT_A - INPUT_A] = LEFT;
    in->mapped_inputs[INPUT_Q - INPUT_A] = UP;
    in->mapped_inputs[INPUT_E - INPUT_A] = DOWN;

    in->control_status = 0;
}

void inputs_key_down(inputs* in, SDL_Keycode key) {
    if(in->mapped_inputs[key - INPUT_A] == 0) {
        return;
    }

    in->control_status |= in->mapped_inputs[key - INPUT_A];
}

void inputs_key_up(inputs* in, SDL_Keycode key) {
    if(in->mapped_inputs[key - SDLK_a] == 0) {
        return;
    }

    in->control_status &= ~in->mapped_inputs[key - INPUT_A];
}
