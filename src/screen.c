#include <buffers.h>
#include <shader.h>
#include <pipeline.h>
#include <screen.h>

static graphics_pipeline pipeline;
static unsigned VAO;

void screen_init(int w, int h) {
    if (w == 0 || h == 0) {
        return;
    }

    vertices v = (vertices){
        .bottom_left_x = 0.0f,
        .bottom_left_y = 0.0f,

        .bottom_right_x = w,
        .bottom_right_y = 0.0f,

        .top_right_x = w,
        .top_right_y = h,

        .top_left_x = 0.0f,
        .top_left_y = h,
    };

    create_vertex_buffer(&VAO, &v);

    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", 0},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    pipeline_init(&pipeline);
    pipeline_compile(2, &pipeline, (shader*[]){&vert_shader, &frag_shader});
}

void screen_render() {
    pipeline_set(&pipeline);
    draw_vertex_buffer(VAO, 3);
}
