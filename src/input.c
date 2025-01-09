#include <input.h>
#include <SDL_mouse.h>

void inputs_init(inputs* in) {
    in->mapped_inputs[INPUT_W - INPUT_A] = FORWARD;
    in->mapped_inputs[INPUT_S - INPUT_A] = BACKWARD;
    in->mapped_inputs[INPUT_D - INPUT_A] = RIGHT;
    in->mapped_inputs[INPUT_A - INPUT_A] = LEFT;
    in->mapped_inputs[INPUT_E - INPUT_A] = UP;
    in->mapped_inputs[INPUT_Q - INPUT_A] = DOWN;

    in->control_status = 0;
}

void inputs_init_callback(InputCallback* callback, InputDirectionCallback* direction_callback) {
    input_direction_callback = direction_callback;
    input_callback = callback;
}

void inputs_key_down(inputs* in, SDL_Keycode key) {
    if(in->control_status & ESCAPE) {
        return;
    }

    if(key == SDLK_ESCAPE) {
        in->control_status = ESCAPE;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        return;
    }

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
    if(in->control_status & ESCAPE) {
        return;
    }

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

// The following gotos are used to skip the gamepad input
// when we can confirm that no gamepad input is present.
void inputs_motion(uint64_t control_status, int x, int y, int dx, int dy) {
    if(control_status == 0) {
        goto skip_to_mouse;
    }

    if(control_status & ESCAPE) {
        return;
    }

    if(control_status < LOOK_UP) {
        goto skip_to_mouse;
    }

    // Gamepad Added later

skip_to_mouse: {
        (*input_direction_callback)(x, y, dx, dy);
    }
}
