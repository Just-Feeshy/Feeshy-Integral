#pragma once

#include <aabb.h>
#include <pipeline.h>
#include <texture.h>
#include <stdbool.h>

typedef struct geometry_pass geometry_pass;

typedef void (*RenderCallback)(void);

struct geometry_pass {
    uint32_t framebuffer;
    uint32_t depth_buffer;
    texture* textures;
    uint32_t texture_count;
    RenderCallback render_callback;
    bool activate_wireframe;
    graphics_pipeline* pipeline;
    AABB aabb;
};

geometry_pass geometry_pass_init(RenderCallback render_callback, int width, int height, int texture_count);
void geometry_pass_add_texture(geometry_pass* pass, texture tex);
void geometry_pass_render(geometry_pass pass, uint32_t texture_offset);
void geometry_pass_destroy(geometry_pass pass);
