#pragma once

#include <texture.h>

typedef void (*RenderCallback)(void);

typedef struct geometry_pass {
    uint32_t framebuffer;
    uint32_t depth_buffer;
    texture texture;
    RenderCallback render_callback;
} geometry_pass;

geometry_pass geometry_pass_init(RenderCallback render_callback, int width, int height);
void geometry_pass_render(geometry_pass pass);
void geometry_pass_destroy(geometry_pass pass);
