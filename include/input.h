#pragma once

#include <utils.h>
#include <stdint.h>
#include <SDL_keycode.h>

// For SDL3
#if SDL_MAJOR_VERSION == 3
#define INPUT_A SDLK_A
#define INPUT_W SDLK_W
#define INPUT_S SDLK_S
#define INPUT_D SDLK_D
#define INPUT_Q SDLK_Q
#define INPUT_E SDLK_E
#else
#define INPUT_A SDLK_a
#define INPUT_W SDLK_w
#define INPUT_S SDLK_s
#define INPUT_D SDLK_d
#define INPUT_Q SDLK_q
#define INPUT_E SDLK_e
#endif

enum controls : uint8_t {
    NONE = 0,
    FORWARD = 1,
    BACKWARD = 2,
    RIGHT = 4,
    LEFT = 8,
    UP = 16,
    DOWN = 32
};

enum control_status : uint8_t {
    PRESS_DOWN = 0,
    PRESS_UP = 1
};

typedef void (*InputCallback)(uint64_t control_status);

typedef struct inputs {
    uint8_t mapped_inputs[INPUT_W - INPUT_A + 1];
    uint64_t control_status;
} inputs;

void inputs_init(inputs* in);
void inputs_init_callback(InputCallback* callback);
void inputs_key_down(inputs* in, SDL_Keycode key);
void inputs_key_up(inputs* in, SDL_Keycode key);
void inputs_update(inputs* in);
