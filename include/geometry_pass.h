#pragma once

#include <aabb.h>
#include <pipeline.h>
#include <texture.h>
#include <stdbool.h>

typedef void (*RenderCallback)(void);

typedef struct geometry_pass {
    uint32_t framebuffer;
    uint32_t depth_buffer;
    texture* textures;
    uint32_t texture_count;
    RenderCallback render_callback;
    bool activate_wireframe;
    graphics_pipeline* pipeline;
    AABB aabb;
} geometry_pass;

geometry_pass geometry_pass_init(RenderCallback render_callback, int width, int height, int texture_count);
void geometry_pass_render(geometry_pass pass);
void geometry_pass_destroy(geometry_pass pass);
