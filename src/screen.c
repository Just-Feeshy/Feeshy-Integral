#include <buffers.h>
#include <shader.h>
#include <menu.h>
#include <utils.h>
#include <program.h>
#include <screen.h>
#include <world.h>
#include <config.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_rwops.h>
#endif

struct fps_counter fps_data;
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
    fprintf(stderr, "Not yet implemented\n");
    #elif FRAGMENT_SELECTOR == 2
    load_shader("shaders/dfao-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #else
    load_shader("shaders/frag-san.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);
    #endif

    pipeline = (graphics_pipeline*)malloc(sizeof(graphics_pipeline));
    pipeline_init(pipeline);
    pipeline_compile(2, pipeline, (shader*[]){&vert_shader, &frag_shader});

    create_constant_location(pipeline, "u_resolution");

    #if defined(HAS_GEOMETRY_PASS) && USE_FBO_WORLD == 1
    create_constant_location(pipeline, "u_volume_tex");
    #endif

#if defined(USE_DFAO) && !defined(HAS_GEOMETRY_PASS)
    create_constant_location(pipeline, "u_volume_tex");
    create_constant_location(pipeline, "u_aabb_min");
    create_constant_location(pipeline, "u_aabb_max");
#endif

    #ifdef HAS_GEOMETRY_PASS
    create_constant_location(pipeline, "u_aabb_min");
    create_constant_location(pipeline, "u_aabb_max");
    #else
    create_constant_location(pipeline, "u_time");
    #endif

    fps_init(&fps_data);
}

void screen_render() {
    pipeline_set(pipeline);

    // Start the benchmark timer for fragment shader
    // Reason why geometry pass is not included is because
    // we are rendering the geometry pass itself
    // not the post processing
    fps_update_begin(&fps_data);

    #if FRAGMENT_SELECTOR == 2
    set_uniform_int("u_volume_tex", 0);
    #endif

    #ifndef HAS_GEOMETRY_PASS
    set_uniform_float("u_time", SDL_GetTicks() / 5000.0f);
    #endif

    set_uniform_vec2("u_resolution", width, height);
    world_setup_uniforms();

    draw_vertex_buffer(VAO, 6);
    glBindVertexArray(0);

    // End the benchmark timer for fragment shader
    fps_update_end(&fps_data);
}
