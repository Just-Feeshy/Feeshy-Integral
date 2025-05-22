#include <gl_dfao.h>
#include <model.h>
#include <uniform_manager.h>
#include <cglm/affine.h>
#include <pipeline.h>
#include <world.h>
#include <timer_query.h>
#include <utils.h>

static Model model;
static AABB aabb;
static Mesh mesh_b;
static Material material_b;
static graphics_pipeline dfao_pipeline;
static texture red_texture = {0};
static int index_mesh = 0;

static void dfao_make_textures(geometry_pass* g_pass, Mesh mesh, AABB aabb) {
    struct GPU_MODULE dfao_module = {0};
    parallelism_init(&dfao_module, "process_MDF", "opencl/mesh_distance_field.cl");

    texture** dfao_textures = (texture**)mem_alloca(sizeof(texture*) * model.mesh_count);
    assert(dfao_textures != NULL);

    dfao_textures[0] = texture_volume(&dfao_module, &mesh, &aabb);
    parallelism_destroy(&dfao_module);

    g_pass->textures = realloc(g_pass->textures, sizeof(texture) * (g_pass->texture_count + 1));
    assert(g_pass->textures != NULL);
    g_pass->textures[g_pass->texture_count] = *dfao_textures[0];
    g_pass->texture_count++;

    // Cleanup
    for (int i = 0; i < model.mesh_count; i++) {
        if (dfao_textures[i]) {
            free(dfao_textures[i]);
        }
    }
}

static void init_bounding_box(Mesh mesh, AABB aabb) {
    red_texture = texture_red_init();
    mesh_b = gen_mesh_cube(aabb);
    material_b = load_material_default();
    material_b.maps[MATERIAL_MAP_ALBEDO].texture = &red_texture;
}

static void draw_bounding_box() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    draw_mesh(mesh_b, material_b);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void dfao_test_world(geometry_pass* g_pass) {
    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", POSITION_ATTR_LOCATION},
        &(shader_attribute){"a_texcoord", TEXCOORD_ATTR_LOCATION},
        &(shader_attribute){"a_normal", NORMAL_ATTR_LOCATION},
        &(shader_attribute){"a_tangent", TANGENT_ATTR_LOCATION},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/baseworld-vert.glsl", &vert_shader, SHADER_VERTEX, 2, vert_attrs);
    load_shader("shaders/baseworld-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    model = load_model("assets/DamagedHelmet.glb");
    glm_translate_make(model.transform, (vec3){0.0f, -2.5f, -5.0f});

    Mesh mesh = model.meshes[index_mesh];
    aabb = get_mesh_AABB(mesh);
    dfao_make_textures(g_pass, mesh, aabb);
    init_bounding_box(mesh, aabb);

    pipeline_init(&dfao_pipeline);
    pipeline_compile(2, &dfao_pipeline, (shader*[]){&vert_shader, &frag_shader});
    create_constant_location(&dfao_pipeline, "u_model");
    create_constant_location(&dfao_pipeline, "u_texture");
}

void dfao_test_world_render() {
    pipeline_set(&dfao_pipeline);


    set_uniform_vec3("u_aabb_min", aabb.min[0], aabb.min[1], aabb.min[2]);
    set_uniform_vec3("u_aabb_max", aabb.max[0], aabb.max[1], aabb.max[2]);

    set_uniform_int("u_texture", 0);
    set_uniform_mat4("u_model", model.transform);
    draw_mesh(model.meshes[index_mesh], model.materials[model.mesh_material[index_mesh]]);
    draw_bounding_box();

}
