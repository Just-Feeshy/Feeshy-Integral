#pragma once

#include <utils.h>
#include <stdint.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_keycode.h>
#else
#include <SDL_keycode.h>
#endif

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

enum controls : uint16_t {
    NONE = 0,
    FORWARD = 1,
    BACKWARD = 2,
    RIGHT = 4,
    LEFT = 8,
    UP = 16,
    DOWN = 32,
    ESCAPE = 64,

    // View controls
    LOOK_UP = 128,
    LOOK_DOWN = 256,
    LOOK_LEFT = 512,
    LOOK_RIGHT = 1024
};

enum control_status : uint8_t {
    PRESS_DOWN = 0,
    PRESS_UP = 1
};

typedef void (*InputCallback)(uint64_t control_status);
typedef void (*InputDirectionCallback)(int x, int y, int dx, int dy);

static volatile InputCallback* input_callback = NULL;
static volatile InputDirectionCallback* input_direction_callback = NULL;

typedef struct inputs {
    uint8_t mapped_inputs[INPUT_W - INPUT_A + 1];
    uint64_t control_status; // In a large game, this would be a u8[128]
} inputs;

void inputs_init(inputs* in);
void inputs_init_callback(InputCallback* callback, InputDirectionCallback* direction_callback);
void inputs_key_down(inputs* in, SDL_Keycode key);
void inputs_key_up(inputs* in, SDL_Keycode key);
void inputs_update(inputs* in);
void inputs_motion(uint64_t control_status, int x, int y, int dx, int dy);
