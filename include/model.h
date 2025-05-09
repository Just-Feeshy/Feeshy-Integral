#pragma once

#include <cglm/mat4.h>
#include <stdint.h>
#include <aabb.h>

// The screen should probably be a mesh.
typedef struct Mesh {
    uint32_t vertex_count;
    uint32_t triangle_count;

    float* vertices;
    float* normals;
    float* tangents;
    float* texcoords;
    float* texcoords2;

    uint16_t* indices;

    uint32_t vaoID;
    uint32_t* vboID;
} Mesh;

typedef struct Model {
    mat4 transform;
    uint32_t mesh_count;
    Mesh* meshes;
} Model;

Model load_model_gltf(const char* path);
void destroy_mesh(Mesh mesh);
void destroy_model(Model* model);
AABB get_mesh_AABB(Mesh mesh);
AABB get_model_AABB(Model model);
