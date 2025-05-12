#define GRID_SIZE 100
#include <model.h>
#include <uniform_manager.h>
#include <cglm/affine.h>

Model model;

void dfao_test_world() {
    // model = load_model("assets/sponza_atrium_3.glb");
    model = load_model("assets/the_utah_teapot.glb");
    glm_translate_make(model.transform, (vec3){0.0f, -50.0f, 200.0f});
}

void dfao_test_world_render() {
    set_uniform_mat4("u_model", model.transform);
    draw_model(model);
}
