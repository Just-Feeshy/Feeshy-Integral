#pragma once

#include <texture.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <stdint.h>
#include <aabb.h>
#include <config.h>

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
    Mesh* meshes;
} Model;

void upload_mesh(Mesh* mesh);
Model load_model(const char* path);
void destroy_mesh(Mesh mesh);
void destroy_model(Model* model);
AABB get_mesh_AABB(Mesh mesh);
AABB get_model_AABB(Model model);
void draw_model(Model model);
void draw_mesh(Mesh mesh);
