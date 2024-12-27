#include <buffers.h>
#include <shader.h>
#include <pipeline.h>
#include <texture.h>
#include <uniform_manager.h>
#include <SDL_rwops.h>
#include <stb_image.h>
#include <program.h>
#include <screen.h>
#include <world.h>

static graphics_pipeline pipeline;
static unsigned VAO;

static int width = 0.0f;
static int height = 0.0f;
static texture txt;

static void load_image(const char* filename, uint8_t** image, int* w, int* h, int* channels) {

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

    *image = stbi_load_from_memory(buffer, size, w, h, channels, 0);
    if (image == NULL) {
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

    uint8_t* image;
    int tex_width, tex_height, tex_channels;
    load_image("assets/image0.jpg", &image, &tex_width, &tex_height, &tex_channels);
    texture_init(&txt, tex_width, tex_height, tex_channels, image);
    stbi_image_free(image);

    load_shader("shaders/vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    pipeline_init(&pipeline);
    pipeline_compile(2, &pipeline, (shader*[]){&vert_shader, &frag_shader});

    uniform_manager_init();
    create_constant_location(&pipeline, "u_resolution");
    create_constant_location(&pipeline, "u_texture");
    world_aspect_ratio(width, height);
}

void screen_render() {
    pipeline_set(&pipeline);
    set_uniform_vec2("u_resolution", width, height);
    set_uniform_int("u_texture", 0);

    world_begin(&pipeline);
    draw_vertex_buffer(VAO, 6);
    world_end(&pipeline);
}
