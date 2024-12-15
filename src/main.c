#include <program.h>
#include <shader.h>
#include <pipeline.h>
#include <screen.h>


static graphics_pipeline pipeline;
static shader frag_shader;
static shader vert_shader;

const int width = 800;
const int height = 600;

int main(int argc, char** argv) {
    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", 0},
    };

    shader_attribute* frag_attrs[] = {
    };

    program_init("Eath", width, height);

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    pipeline_init(&pipeline);
    pipeline_compile(2, &pipeline, (shader*[]){&vert_shader, &frag_shader});
    //screen_init(width, height);

    program_loop();
    program_destroy();
    return 0;
}
