#include <buffers.h>
#include <shader.h>
#include <menu.h>
#include <utils.h>
#include <program.h>
#include <screen.h>
#include <world.h>
#include <timer_query.h>
#include <config.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_rwops.h>
#endif

graphics_pipeline* pipeline;

uint32_t ms_time_elapsed = 0;
uint16_t data_update_iteration = 0;
uint64_t accumulated_time = 0;

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
        &(shader_attribute){"a_position", POSITION_ATTR_LOCATION},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);

    #if FRAGMENT_SELECTOR == 1
    load_shader("shaders/frag-sec.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #elif FRAGMENT_SELECTOR == 2
    load_shader("shaders/dfao-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #else
    load_shader("shaders/frag-san.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #endif

    pipeline = (graphics_pipeline*)malloc(sizeof(graphics_pipeline));
    pipeline_init(pipeline);
    pipeline_compile(2, pipeline, (shader*[]){&vert_shader, &frag_shader});

    create_constant_location(pipeline, "u_resolution");

    #if FRAGMENT_SELECTOR == 2
    create_constant_location(pipeline, "u_texture");
    create_constant_location(pipeline, "u_volume_tex");
    create_constant_location(pipeline, "u_aabb_min");
    create_constant_location(pipeline, "u_aabb_max");
    #else
    create_constant_location(pipeline, "u_time");
    #endif

    gpu_timer_query_init();
}

void screen_render() {
    pipeline_set(pipeline);

    // Start the benchmark timer for fragment shader
    // Reason why geometry pass is not included is because
    // we are rendering the geometry pass itself
    // not the post processing

    gpu_timer_query_begin();

    #if FRAGMENT_SELECTOR == 2
    set_uniform_int("u_volume_tex", 1);
    #else
    set_uniform_float("u_time", SDL_GetTicks() / 5000.0f);
    #endif

    set_uniform_vec2("u_resolution", width, height);
    world_setup_uniforms();

    draw_vertex_buffer(VAO, 6);

    // End the benchmark timer for fragment shader
    gpu_timer_query_end();

    #ifndef HAS_GEOMETRY_PASS
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
    #endif
}
