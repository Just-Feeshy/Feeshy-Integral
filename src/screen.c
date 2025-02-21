#include <buffers.h>
#include <shader.h>
#include <pipeline.h>
#include <texture.h>
#include <uniform_manager.h>
#include <SDL_rwops.h>
#include <stb_image.h>
#include <menu.h>
#include <program.h>
#include <screen.h>
#include <world.h>

#define IMAGES 2

uint64_t ms_time_elapsed = 0;


static GLuint query;
static graphics_pipeline pipeline;
static unsigned VAO;

static int width = 0.0f;
static int height = 0.0f;
static texture txt[IMAGES];

static void load_image(const char* filename, image* img) {

    SDL_RWops* file = SDL_RWFromFile(filename, "rb");
    if(file == NULL) {
        char out_filename[strlen(filename) + 4]; // Stack based buffer
        strcpy(out_filename, "../");
        strcat(out_filename, filename);

        file = SDL_RWFromFile(out_filename, "rb");
    }

    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    int64_t size = SDL_RWsize(file);
    uint8_t* buffer = (uint8_t*)malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        goto file_error;
    }

    if (size == -1) {
        fprintf(stderr, "Failed to get file size: %s\n", filename);
        goto file_error;
    }

    if (SDL_RWread(file, buffer, 1, size) != size) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        goto file_error;
    }

    img->data = stbi_load_from_memory(buffer, size, &img->width, &img->height, &img->channels, 0);
    if (img == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        goto file_error;
    }

file_error:
    SDL_RWclose(file);
    free(buffer);
}

void screen_init(int w, int h) {
    if (w == 0 || h == 0) {
        return;
    }

    width = (float)w * program_get_pixel_density();
    height = (float)h * program_get_pixel_density();

    vertices v = (vertices){
        .bottom_left_x = 0.0f,
        .bottom_left_y = 0.0f,

        .bottom_right_x = width,
        .bottom_right_y = 0.0f,

        .top_right_x = width,
        .top_right_y = height,

        .top_left_x = 0.0f,
        .top_left_y = height,
    };

    create_vertex_buffer(&VAO, &v);

    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", 0},
    };

    shader_attribute* frag_attrs[] = {
    };

    image img[IMAGES];

    load_image("assets/8k_saturn.jpg", &img[0]);
    img[1] = *create_simplex_noise(1 << (7 + selected_menu_item), 1 << (7 + selected_menu_item));

    texture_init(&txt[0], &img[0]);
    texture_init(&txt[1], &img[1]);

    for (int i = 0; i < IMAGES; i++) {
        stbi_image_free(img[i].data);
    }

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    pipeline_init(&pipeline);
    pipeline_compile(2, &pipeline, (shader*[]){&vert_shader, &frag_shader});

    uniform_manager_init();
    create_constant_location(&pipeline, "u_resolution");
    create_constant_location(&pipeline, "u_time");
    create_constant_location(&pipeline, "u_quality");
    create_constant_location(&pipeline, "u_texture0");
    create_constant_location(&pipeline, "u_texture1");
    world_aspect_ratio(width, height);

    glGenQueries(1, &query);
}

void screen_render() {
    // Start the benchmark timer for fragment shader
    glBeginQuery(GL_TIME_ELAPSED, query);

    pipeline_set(&pipeline);
    texture_bind(&txt[0], 0);
    texture_bind(&txt[1], 1);
    set_uniform_vec2("u_resolution", width, height);
    set_uniform_int("u_quality", 1 << (7 + selected_menu_item));
    set_uniform_float("u_time", SDL_GetTicks() / 5000.0f);
    set_uniform_int("u_texture0", 0);
    set_uniform_int("u_texture1", 1);

    world_begin(&pipeline);
    draw_vertex_buffer(VAO, 6);
    world_end(&pipeline);

    // End the benchmark timer for fragment shader
    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 timeElapsed = 1;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);

    ms_time_elapsed = timeElapsed / 1e5;
}
