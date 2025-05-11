#include <buffers.h>
#include <shader.h>
#include <pipeline.h>
#include <uniform_manager.h>
#include <stb_image.h>
#include <menu.h>
#include <program.h>
#include <screen.h>
#include <world.h>
#include <timer_query.h>
#include <config.h>

#if FRAGMENT_SELECTOR == 2
#include <gl_dfao.h>
#endif

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_rwops.h>
#endif

#define MAX_ITERATIONS 10000
#define IMAGES 2

uint32_t ms_time_elapsed = 0;

uint16_t data_update_iteration = 0;
uint64_t accumulated_time = 0;

static graphics_pipeline pipeline;
static unsigned VAO;

static int width = 0.0f;
static int height = 0.0f;

static vertices v;

void screen_init(int w, int h) {
    if (w == 0 || h == 0) {
        return;
    }

    width = (float)w * program_get_pixel_density();
    height = (float)h * program_get_pixel_density();

    v = (vertices){
        .bottom_left_x = 0.0f,
        .bottom_left_y = 0.0f,

        .bottom_right_x = width,
        .bottom_right_y = 0.0f,

        .top_right_x = width,
        .top_right_y = height,

        .top_left_x = 0.0f,
        .top_left_y = height,
    };

    create_vertex_buffer(&VAO, v);

    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", 0},
    };

    shader_attribute* frag_attrs[] = {
    };

    printf("Fragment Selector: %d\n", FRAGMENT_SELECTOR);

    #if FRAGMENT_SELECTOR == 1
    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag-sec.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #elif FRAGMENT_SELECTOR == 2
    #else
    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag-san.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #endif

    pipeline_init(&pipeline);
    pipeline_compile(2, &pipeline, (shader*[]){&vert_shader, &frag_shader});

    uniform_manager_init();
    create_constant_location(&pipeline, "u_resolution");
    create_constant_location(&pipeline, "u_time");
    gpu_timer_query_init();
}

void screen_render() {
    // Start the benchmark timer for fragment shader
    gpu_timer_query_begin();

    pipeline_set(&pipeline);
    set_uniform_vec2("u_resolution", width, height);
    set_uniform_float("u_time", SDL_GetTicks() / 5000.0f);

    world_begin(&pipeline);
    draw_vertex_buffer(VAO, 6);
    world_end(&pipeline);

    // End the benchmark timer for fragment shader
    gpu_timer_query_end();
    ms_time_elapsed = gpu_timer_query_result();

    if(data_update_iteration < MAX_ITERATIONS) {
        accumulated_time += ms_time_elapsed;
        data_update_iteration++;
    }

    #ifndef EMSCRIPTEN
    if(data_update_iteration == MAX_ITERATIONS) {
        printf("Average time: %llu\n", accumulated_time);
        data_update_iteration++;
    }
    #endif
}
