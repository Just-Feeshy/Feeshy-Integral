#define IS_OPENCL
#include <aabb.h>

// Couldn't be bothered to write my closest point on triangle function
// so I just copied it from the internet via stackoverflow.
// https://stackoverflow.com/questions/2924795/fastest-way-to-compute-point-to-triangle-distance-in-3d
static inline float3 closestPointOnTriangle(float3 p, float3 a, float3 b, float3 c) {
    float3 ab = b - a;
    float3 ac = c - a;
    float3 ap = p - a;

    float d1 = dot(ab, ap);
    float d2 = dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;

    float3 bp = p - b;
    float d3 = dot(ab, bp);
    float d4 = dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;

    float vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return a + v * ab;
    }

    float3 cp = p - c;
    float d5 = dot(ab, cp);
    float d6 = dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;

    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + w * ac;
    }

    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b);
    }

    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

// Originally I had this be the closest point on triangle function,
// but I ended up rewriting it and such I separated the two functions.
static inline float pointTriangleDistance(float3 p, float3 a, float3 b, float3 c) {
    float3 cp = closestPointOnTriangle(p, a, b, c);
    return length(p - cp);
}

static inline bool rayIntersectsTriangle(
    float3 orig, float3 dir,
    float3 v0, float3 v1, float3 v2,
    float* outT, float3* outNormal)
{
    const float EPSILON = 1e-5f;
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;

    float3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    if (fabs(a) < EPSILON) return false; // Ray parallel to triangle

    float f = 1.0f / a;
    float3 s = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    float3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * dot(edge2, q);
    if (t < 0.0f) return false;

    // It's a hit
    *outT = t;
    if (outNormal != NULL) {
        *outNormal = normalize(cross(edge1, edge2));
    }

    return true;
}

__kernel void process_MDF(
    __global float* distance_field,
    __constant float* vertices,
    __constant float* normals,
    __constant short* indices,
    int vertex_count,
    int triangle_count,
    __constant AABB* mesh_bound
) {
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
    float3 normal = (float3)(0.0f, 0.0f, 0.0f);
    bool hit = false;
    int hit_count = 0;
    float3 direction = (float3)(0.0f, 0.0f, 1.0f);

    // Very crappy, unoptimized distance field calculation
    // Ironic since this research is about optimizations of distance fields
    for(int i=0; i<triangle_count; i++) {
        short3 triangle = {
            indices[i * 3],
            indices[i * 3 + 1],
            indices[i * 3 + 2]
        };

        float3 a = {
            vertices[triangle.x * 3],
            vertices[triangle.x * 3 + 1],
            vertices[triangle.x * 3 + 2]
        };

        float3 b = {
            vertices[triangle.y * 3],
            vertices[triangle.y * 3 + 1],
            vertices[triangle.y * 3 + 2]
        };

        float3 c = {
            vertices[triangle.z * 3],
            vertices[triangle.z * 3 + 1],
            vertices[triangle.z * 3 + 2]
        };

        float distance = pointTriangleDistance(point, a, b, c);
        min_distance = fmin(min_distance, distance);

        float t;
        float3 cur_normal = (float3)(0.0f, 0.0f, 0.0f);
        if (rayIntersectsTriangle(point, direction, a, b, c, &t, &cur_normal)
        && t < min_t) {
            hit = true;
            min_t = t;
            normal = cur_normal;
            hit_count++;
        }
    }

    if(hit && dot(normal, direction) > 0.0f) {
        min_distance = -min_distance;
    }

    int index = x
            + y * get_global_size(0)
            + z * get_global_size(0) * get_global_size(1);
    distance_field[index] = min_distance;
}
