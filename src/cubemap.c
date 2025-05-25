#include <cubemap.h>
#include <model.h>
#include <pipeline.h>
#include <uniform_manager.h>

static Mesh cubemap_mesh;
static Material cubemap_material;

void cubemap_create_mesh(texture* tex) {
    AABB bounds = {
        .min = {-10.0f, -10.0f, -10.0f},
        .max = {10.0f, 10.0f, 10.0f},
    };

    image main_image = load_image("assets/Daylight.png");
    cubemap_mesh = gen_mesh_cube(bounds);
    cubemap_material = load_material_default();
    cubemap_material.maps[MATERIAL_MAP_ALBEDO].texture = tex;
    assert(cubemap_material.maps[MATERIAL_MAP_ALBEDO].texture != NULL);
}

void cubemap_init(geometry_pass* g_pass) {
    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", POSITION_ATTR_LOCATION},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/skybox-vert.glsl", &vert_shader, SHADER_VERTEX, 1, vert_attrs);
    load_shader("shaders/skybox-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    image main_image = load_image("assets/Daylight.png");
    image faces[6] = {
        extract_face(main_image, 2, 1),
        extract_face(main_image, 0, 1),
        extract_face(main_image, 1, 0),
        extract_face(main_image, 1, 2),
        extract_face(main_image, 1, 1),
        extract_face(main_image, 3, 1),
    };

    // Memory leak here, but we will free it later
    texture* cubemap_texture = texture_cubemap_init(faces);

    for(int i=0; i<6; i++) {
        free(faces[i].data);
    }

    cubemap_create_mesh(cubemap_texture);
    pipeline_compile(2, g_pass->pipeline, (shader*[]){&vert_shader, &frag_shader});
    create_constant_location(g_pass->pipeline, "u_texture");
}

void cubemap_mesh_render() {
    draw_mesh(cubemap_mesh, cubemap_material);
}

void cubemap_render() {
    set_uniform_int("u_texture", 0);
    cubemap_mesh_render();
}
