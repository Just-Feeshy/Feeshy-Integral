#define GRID_SIZE 100
#include <model.h>
#include <uniform_manager.h>
#include <cglm/affine.h>
#include <pipeline.h>
#include <world.h>
#include <timer_query.h>

Model model;
graphics_pipeline dfao_pipeline;

void dfao_test_world() {
    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", POSITION_ATTR_LOCATION},
        &(shader_attribute){"a_texcoord", TEXCOORD_ATTR_LOCATION},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/dfao-vert.glsl", &vert_shader, SHADER_VERTEX, 2, vert_attrs);
    load_shader("shaders/dfao-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    model = load_model("assets/sponza_atrium_3.glb");
    glm_translate_make(model.transform, (vec3){0.0f, -2.5f, -5.0f});
    // model = load_model("assets/the_utah_teapot.glb");

    pipeline_init(&dfao_pipeline);
    pipeline_compile(2, &dfao_pipeline, (shader*[]){&vert_shader, &frag_shader});
    create_constant_location(&dfao_pipeline, "u_model");
}

void dfao_test_world_render() {
    pipeline_set(&dfao_pipeline);

    gpu_timer_query_begin();
    world_begin(&dfao_pipeline);

    set_uniform_mat4("u_model", model.transform);
    draw_model(model);

    world_end(&dfao_pipeline);
    gpu_timer_query_end();
}
