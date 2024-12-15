#include <program.h>
#include <shader.h>

static shader_attribute* vert_attrs[] = {
    &(shader_attribute){"a_position", 0},
};

static shader_attribute* frag_attrs[] = {
};

static shader frag_shader;
static shader vert_shader;

int main(int argc, char** argv) {
    program_init("Eath", 800, 600);

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    program_loop();
    program_destroy();
    return 0;
}
