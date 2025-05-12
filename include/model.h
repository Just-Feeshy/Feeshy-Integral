#pragma once

#include <texture.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <stdint.h>
#include <aabb.h>
#include <config.h>

// Taken from Raylib
// Never planned on having a model loader, but here we are.
// If I did, I would have just used Raylib itself.

typedef enum {
    MATERIAL_MAP_ALBEDO = 0,        // Albedo material (same as: MATERIAL_MAP_DIFFUSE)
    MATERIAL_MAP_METALNESS,         // Metalness material (same as: MATERIAL_MAP_SPECULAR)
    MATERIAL_MAP_NORMAL,            // Normal material
    MATERIAL_MAP_ROUGHNESS,         // Roughness material
    MATERIAL_MAP_OCCLUSION,         // Ambient occlusion material
    MATERIAL_MAP_EMISSION,          // Emission material
    MATERIAL_MAP_HEIGHT,            // Heightmap material
    MATERIAL_MAP_CUBEMAP,           // Cubemap material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_IRRADIANCE,        // Irradiance material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_PREFILTER,         // Prefilter material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_BRDF               // Brdf material
} MaterialMapIndex;

#define MATERIAL_MAP_DIFFUSE MATERIAL_MAP_ALBEDO
#define MATERIAL_MAP_SPECULAR MATERIAL_MAP_METALNESS

typedef struct MaterialMap {
    texture* texture;
    float value;
} MaterialMap;

typedef struct Material {
    MaterialMap* maps;
} Material;

// The screen should probably be a mesh.
typedef struct Mesh {
    uint32_t vertex_count;
    uint32_t triangle_count;

    float* vertices;
    float* normals;
    float* tangents;
    float* texcoords;
    float* texcoords2;

    #ifdef SUPPORT_32_BIT_INDICES
    uint32_t* indices;
    #else
    uint16_t* indices;
    #endif

    uint32_t vaoID;
    uint32_t* vboID;
} Mesh;

typedef struct Model {
    mat4 transform;
    uint32_t mesh_count;
    uint32_t material_count;
    int* mesh_material;
    Material* materials;
    Mesh* meshes;
} Model;

Material load_material_default();
void upload_mesh(Mesh* mesh);
Model load_model(const char* path);
void destroy_mesh(Mesh mesh);
void destroy_model(Model* model);
AABB get_mesh_AABB(Mesh mesh);
AABB get_model_AABB(Model model);
void draw_model(Model model);
void draw_mesh(Mesh mesh, Material material);
