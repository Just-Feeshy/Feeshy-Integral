#pragma once

#include <config.h>

#define UNIFORM_GRID_X GRID_SIZE
#define UNIFORM_GRID_Y GRID_SIZE
#define UNIFORM_GRID_Z GRID_SIZE

typedef float vec3[3];

enum {
    UNIFORM_GRID_SIZE = UNIFORM_GRID_X * UNIFORM_GRID_Y * UNIFORM_GRID_Z
};

typedef struct Mesh {
    int vertex_count;
    int triangle_count;

    float* vertices;
    float* normals;
    float* tangents;
    float* texcoords;
    float* texcoords2;

    unsigned short* indices;

    unsigned int vaoID;
    unsigned int* vboID;
    unsigned int* gvID;
} Mesh;
