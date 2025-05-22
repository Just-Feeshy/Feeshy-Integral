#pragma once

#include <texture.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <stdint.h>
#include <mesh.h>
#include <aabb.h>
#include <config.h>

// Taken from Raylib
// Never planned on having a model loader, but here we are.
// If I did, I would have just used Raylib itself.

typedef enum {
    MATERIAL_MAP_ALBEDO = 0,        // Albedo material (same as: MATERIAL_MAP_DIFFUSE)
    MATERIAL_MAP_OCCLUSION = 1,         // Ambient occlusion material
    MATERIAL_MAP_NORMAL = 2,            // Normal material
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
Mesh gen_mesh_cube(AABB aabb);
Model load_model(const char* path);
void destroy_mesh(Mesh mesh);
void destroy_model(Model* model);
AABB get_mesh_AABB(Mesh mesh);
AABB get_model_AABB(Model model, AABB* out_mesh_aabb[]);
void draw_model(Model model);
void draw_mesh(Mesh mesh, Material material);
