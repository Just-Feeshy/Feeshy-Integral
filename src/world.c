#include <stdint.h>
#include <world.h>
#include <light.h>
#include <menu.h>
#include <cam_matrices.h>
#include <cglm/vec3.h>
#include <uniform_block_state.h>
#include <uniform_manager.h>
#include <screenshot.h>
#include <program.h>
#include <input.h>
#include <config.h>

/*
 * One of the two source files with spaghetti code
 * Specifically for macros
 * I don't care about the code quality here
 * just wanted more easy control over the lexical scope
 * and avoid commenting a lot of code and uncommenting it later
*/

#ifdef USE_DFAO
#include <gl_dfao.h>
#endif

#ifdef HAS_GEOMETRY_PASS
    #if USE_FBO_WORLD == 1
    static geometry_pass g_pass;
    #endif
#endif

#ifndef EMSCRIPTEN
#define SPEED 0.1
#else
#define SPEED 0.2
#endif

#define TAU 6.28318530718

static const float LIMIT = TAU / 4.01;

// Camera

static cam_matrices cam;
static uniform_block ubo;
static sized_shader_block*** block; // This is so ugly, but it works and I'm tired, got only 2 hours of sleep last night
static float aspect_ratio;

static void render_cam() {
    update_rotation(&cam);
    update_view_matrix(&cam);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
}

static void world_input_callback_impl(uint64_t control_status, input_status input) {
    if(control_status & FORWARD) {
        vec3 front_scaled;
        glm_vec3_scale(cam.front, SPEED, front_scaled);
        glm_vec3_add(cam.cam.position, front_scaled, cam.cam.position);
    }

    if(control_status & BACKWARD) {
        vec3 backward_scaled;
        glm_vec3_scale(cam.front, SPEED, backward_scaled);
        glm_vec3_sub(cam.cam.position, backward_scaled, cam.cam.position);
    }

    if(control_status & RIGHT) {
        vec3 right_scaled;
        glm_vec3_scale(cam.right, SPEED, right_scaled);
        glm_vec3_add(cam.cam.position, right_scaled, cam.cam.position);
    }

    if(control_status & LEFT) {
        vec3 left_scaled;
        glm_vec3_scale(cam.right, SPEED, left_scaled);
        glm_vec3_sub(cam.cam.position, left_scaled, cam.cam.position);
    }

    if(control_status & UP) {
        vec3 up_scaled;
        glm_vec3_scale(cam.up, SPEED, up_scaled);
        glm_vec3_add(cam.cam.position, up_scaled, cam.cam.position);
    }

    if(control_status & DOWN) {
        vec3 down_scaled;
        glm_vec3_scale(cam.up, SPEED, down_scaled);
        glm_vec3_sub(cam.cam.position, down_scaled, cam.cam.position);
    }

    #ifndef EMSCRIPTEN
    if(control_status & SCREENSHOT && input == PRESS_DOWN) {
        printf("Screenshot Saved!\n");
        capture_screenshot();
    }
    #endif

    render_cam();
}

static void world_direction_callback_impl(int x, int y, int dx, int dy) {

    cam.horizontal_angle = fmod(cam.horizontal_angle - dx / 512.0, TAU);
    cam.vertical_angle = fclamp(cam.vertical_angle - dy / 512.0, -LIMIT, LIMIT);
    render_cam();
}

void world_init(int w, int h) {
    printf("World Initialized\n");

#ifdef HAS_GEOMETRY_PASS
    #if USE_FBO_WORLD == 1 && defined(USE_DFAO)
    printf("Using Geometry Pass with FBO\n");
    RenderCallback dfao_callback = dfao_test_world_render;

    g_pass = geometry_pass_init(
        dfao_test_world_render,
        w * program_get_pixel_density(),
        h * program_get_pixel_density(),
        1
    );

    dfao_test_world(&g_pass);
    #endif
#endif

#if defined(USE_DFAO) && !defined(HAS_GEOMETRY_PASS)
    dfao_test_world();
#endif

    static InputCallback world_input_callback = world_input_callback_impl;
    static InputDirectionCallback world_direction_callback = world_direction_callback_impl;
    inputs_init_callback(&world_input_callback, &world_direction_callback);
    init_ubo(&ubo);

    cam = create_cam_matrices();
    init_cam_matrices(&cam);

    block = (sized_shader_block***)malloc(sizeof(sized_shader_block**) * 2);
    *block = (sized_shader_block**)malloc(sizeof(sized_shader_block*));
    **block = create_ssbo(&ubo, GL_UNIFORM_BUFFER, sizeof(cam_matrices));

    *(block + 1) = (sized_shader_block**)malloc(sizeof(sized_shader_block*));
    render_cam();
}

// This sets up the basic uniforms for the world
// aka. uniforms used in the world
void world_setup_uniforms() {
#ifdef HAS_GEOMETRY_PASS
    #if USE_FBO_WORLD == 1
    set_uniform_vec3("u_aabb_min", g_pass.aabb.min[0], g_pass.aabb.min[1], g_pass.aabb.min[2]);
    set_uniform_vec3("u_aabb_max", g_pass.aabb.max[0], g_pass.aabb.max[1], g_pass.aabb.max[2]);
    #endif
#endif
}

void world_aspect_ratio(float width, float height) {
    aspect_ratio = width / height;

    update_projection_matrix(&cam, aspect_ratio, 45.0f);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
    glm_vec3_copy(cam.init_position, cam.cam.position);
}


/*
 * Don't care this spaghetti macro code
 * especially for it repeating twice
 * I just want something that works
 * and is dynamic
*/

void world_begin(graphics_pipeline* pipe) {
    size_t pipe_count = 1;

    #ifdef HAS_GEOMETRY_PASS
    graphics_pipeline* pipelines[] = {
        pipe,

        #if USE_FBO_WORLD == 1
        g_pass.pipeline,
        #endif
    };

    pipe_count = 1 + USE_FBO_WORLD;
    #else
    graphics_pipeline* pipelines[] = {pipe};
    #endif

    bind_ubo_with_name(&ubo, "CamBlock", *block, pipelines, pipe_count);
    size_t texture_count = 0;

    #if defined(HAS_GEOMETRY_PASS)
        #if USE_FBO_WORLD == 1
    geometry_pass_render(g_pass, texture_count);
    texture_count += g_pass.texture_count;
        #endif
    #endif
}

void world_end(graphics_pipeline* pipe) {
    #ifdef USE_DFAO
    dfao_test_world_render();
    #endif

    size_t pipe_count = 1;

    #ifdef HAS_GEOMETRY_PASS
    graphics_pipeline* pipelines[] = {
        pipe,

        #if USE_FBO_WORLD == 1
        g_pass.pipeline,
        #endif
    };

    pipe_count = 1 + USE_FBO_WORLD;
    #else
    graphics_pipeline* pipelines[] = {pipe};
    #endif

    unbind_ubo(&ubo, 0, **block, pipelines, pipe_count);
}


// Event Related

void world_reset_camera() {
    cam.horizontal_angle = 0.0f;
    cam.vertical_angle = 0.0f;

    glm_vec3_copy(cam.init_position, cam.cam.position);
    render_cam();
}

void world_update_fov(float fov) {
    update_projection_matrix(&cam, aspect_ratio, fov);
    set_ssbo_data(**block, &cam.cam, sizeof(cam_block));
}

void world_toggle_wireframe() {
    #if defined(HAS_GEOMETRY_PASS) && USE_FBO_WORLD == 1
    g_pass.activate_wireframe = !g_pass.activate_wireframe;
    #endif
}

void world_destroy() {
#ifdef HAS_GEOMETRY_PASS
    #if USE_FBO_WORLD == 1
    geometry_pass_destroy(g_pass);
    #endif
#endif

    free(block[0]);
    free(block[1]);
    free(block);
}
