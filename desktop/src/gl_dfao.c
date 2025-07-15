#include <gl_dfao.h>
#include <model.h>
#include <uniform_manager.h>
#include <cglm/affine.h>
#include <world.h>
#include <utils.h>

#define EPSILON 0.25f

#if defined(USE_DFAO) && !defined(HAS_GEOMETRY_PASS)
texture* mesh_volume_texture = 0;
#endif

static Model model;
static Mesh mesh_b;
static AABB mesh_aabb;
static Material material_b;
static texture red_texture = {0};
static int index_mesh = 0;

static void dfao_make_textures(geometry_pass* g_pass, Mesh mesh, AABB aabb, mat4 transform) {
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
    for(int i = 0; i < model.mesh_count; i++) {
        if (dfao_textures[i]) {
            free(dfao_textures[i]);
        }
    }
}

void dfao_test_world(
#if HAS_GEOMETRY_PASS
    geometry_pass* g_pass
#endif
) {
    model = load_model("assets/DamagedHelmet.glb");

    mat4 rot;
    glm_rotate_make(rot, 180.0f, (vec3){1.0f, 0.0f, 0.0f});
    glm_mat4_mul(rot, model.transform, model.transform);

    Mesh mesh = model.meshes[index_mesh];
    mesh_aabb = get_mesh_AABB(mesh);
    glm_vec3_add(mesh_aabb.min, (vec3){-EPSILON, -EPSILON, -EPSILON}, mesh_aabb.min);
    glm_vec3_add(mesh_aabb.max, (vec3){EPSILON, EPSILON, EPSILON}, mesh_aabb.max);

#ifdef HAS_GEOMETRY_PASS
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

    g_pass->aabb = mesh_aabb;
    dfao_make_textures(g_pass, mesh, g_pass->aabb, model.transform);
    pipeline_compile(2, g_pass->pipeline, (shader*[]){&vert_shader, &frag_shader});
    create_constant_location(g_pass->pipeline, "u_model");
    create_constant_location(g_pass->pipeline, "u_texture");
#elif defined(USE_DFAO) && !defined(HAS_GEOMETRY_PASS)
    struct GPU_MODULE dfao_module = {0};
    parallelism_init(&dfao_module, "process_MDF", "opencl/mesh_distance_field.cl");
    mesh_volume_texture = texture_volume(&dfao_module, &mesh, &mesh_aabb);
    parallelism_destroy(&dfao_module);

    texture_bind(mesh_volume_texture, 0);
#endif
}

void dfao_test_world_render() {
#if defined(USE_DFAO) && !defined(HAS_GEOMETRY_PASS)
    set_uniform_vec3("u_aabb_min", mesh_aabb.min[0], mesh_aabb.min[1], mesh_aabb.min[2]);
    set_uniform_vec3("u_aabb_max", mesh_aabb.max[0], mesh_aabb.max[1], mesh_aabb.max[2]);
#else
    draw_model(model);
#endif
}
