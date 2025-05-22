#define IS_OPENCL
#include <aabb.h>

__kernel void process_MDF(__global float* distance_field, __constant float* vertices, __constant float* normals, int vertex_count, __constant AABB* mesh_bound) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int z = get_global_id(2);

    float3 cell_size = (float3)(
        (mesh_bound->max[0] - mesh_bound->min[0]) / get_global_size(0),
        (mesh_bound->max[1] - mesh_bound->min[1]) / get_global_size(1),
        (mesh_bound->max[2] - mesh_bound->min[2]) / get_global_size(2)
    );

    float3 point = (float3)(
        (float)x * cell_size.x + mesh_bound->min[0],
        (float)y * cell_size.y + mesh_bound->min[1],
        (float)z * cell_size.z + mesh_bound->min[2]
    );

    float min_distance = FLT_MAX;
    float min_t = FLT_MAX;
    int intersected_point = 0;
    float3 direction = (float3)(0.0f, 0.0f, 1.0f);
    bool is_hit = false;

    // Very crappy, unoptimized distance field calculation
    // Ironic since this research is about optimizations of distance fields
    for(int i=0; i<vertex_count; i++) {
        float3 vertex = (float3)(
            vertices[3*i],
            vertices[3*i+1],
            vertices[3*i+2]
        );

        float distance = length(point - vertex);
        min_distance = min(min_distance, distance);

        float t = dot((vertex - point) / distance, direction);
        if(t != 1.0) {
            continue;
        }

        if(t < min_t) {
            is_hit = true;
            intersected_point = i;
            min_t = t;
        }
    }

    float3 mesh_normal = {
        normals[3*intersected_point],
        normals[3*intersected_point+1],
        normals[3*intersected_point+2]
    };

    if(is_hit && dot(mesh_normal, direction) > 0.0f) {
        min_distance = -min_distance;
    }

    int index = x
            + y * get_global_size(0)
            + z * get_global_size(0) * get_global_size(1);
    distance_field[index] = min_distance;
}
