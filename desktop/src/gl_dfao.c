#include <gl_dfao.h>
#include <model.h>
#include <uniform_manager.h>
#include <cglm/affine.h>
#include <world.h>
#include <utils.h>

#define EPSILON 0.1f

static Model model;
static Mesh mesh_b;
static Material material_b;
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

void dfao_test_world(geometry_pass* g_pass) {
    shader frag_shader;
    shader vert_shader;

    shader_attribute* vert_attrs[] = {
        &(shader_attribute){"a_position", POSITION_ATTR_LOCATION},
        &(shader_attribute){"a_texcoord", TEXCOORD_ATTR_LOCATION},
    };

    shader_attribute* frag_attrs[] = {
    };

    load_shader("shaders/baseworld-vert.glsl", &vert_shader, SHADER_VERTEX, 2, vert_attrs);
    load_shader("shaders/baseworld-frag.glsl", &frag_shader, SHADER_FRAGMENT, 0, frag_attrs);

    model = load_model("assets/DamagedHelmet.glb");

    Mesh mesh = model.meshes[index_mesh];
    g_pass->aabb = get_mesh_AABB(mesh);
    glm_vec3_add(g_pass->aabb.min, (vec3){-EPSILON, -EPSILON, -EPSILON}, g_pass->aabb.min);
    glm_vec3_add(g_pass->aabb.max, (vec3){EPSILON, EPSILON, EPSILON}, g_pass->aabb.max);
    dfao_make_textures(g_pass, mesh, g_pass->aabb);

    pipeline_compile(2, g_pass->pipeline, (shader*[]){&vert_shader, &frag_shader});
    create_constant_location(g_pass->pipeline, "u_model");
    create_constant_location(g_pass->pipeline, "u_texture");
}

void dfao_test_world_render() {
    set_uniform_int("u_texture", 0);
    set_uniform_mat4("u_model", model.transform);
    draw_model(model);
}
