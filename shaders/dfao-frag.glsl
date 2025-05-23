#line 2

precision mediump float;

#define TAU 6.28318530

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform sampler2D u_texture;
uniform sampler3D u_volume_tex;
uniform vec3 u_aabb_min;
uniform vec3 u_aabb_max;
uniform vec2 u_resolution;

in vec2 v_position;

const vec3 light_pos = vec3(0.0, 20.0, 0.0);

#define MAX_STEPS 99
#define NEW_RAYMARCH 0

// Thank you for "A Minimal Ray-Tracer"
// The original code can be found at:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
bool intersectBox(vec3 ro, vec3 rd, out float t0, out float t1) {
    vec3 bounds[2] = vec3[2](u_aabb_min, u_aabb_max);
    vec3 inv_dir = 1.0 / rd;
    ivec3 sign = ivec3(// Better than using the `step` function
        (inv_dir.x < 0.0),
        (inv_dir.y < 0.0),
        (inv_dir.z < 0.0)
    );

    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    tmin = (bounds[sign.x].x - ro.x) * inv_dir.x;
    tmax = (bounds[1 - sign.x].x - ro.x) * inv_dir.x;
    tymin = (bounds[sign.y].y - ro.y) * inv_dir.y;
    tymax = (bounds[1 - sign.y].y - ro.y) * inv_dir.y;

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin) {
        tmin = tymin;
    }

    if (tymax < tmax) {
        tmax = tymax;
    }

    tzmin = (bounds[sign.z].z - ro.z) * inv_dir.z;
    tzmax = (bounds[1 - sign.z].z - ro.z) * inv_dir.z;

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    if (tzmin > tmin) {
        tmin = tzmin;
    }

    if (tzmax < tmax) {
        tmax = tzmax;
    }

    t0 = tmin;
    t1 = tmax;

    if(t0 < 0.0 && t1 >= 0.0) {
        t0 = 0.01;
    }

    if (t1 < 0.0) {
        return false;
    }

    return true;
}

vec3 get_tex_coord(vec3 pos) {
    return (pos - u_aabb_min) / (u_aabb_max - u_aabb_min);
}

float sampleDistance(vec3 pos) {
    vec3 tex_coord = get_tex_coord(pos);
    return max(texture(u_volume_tex, tex_coord).r, 0.0);
}

// Basically a hemisphere sampling
// I literally learned this from my Calc III class
// So it was pretty easy to implement
vec3 sampleHemisphere(vec3 normal, int i, int total) {
    float phi = TAU * float(i) / float(total); // full circle
    float cosTheta = float(i + 0.5) / float(total);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Local tangent space (TBN)
    vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);

    // Spherical to Cartesian
    vec3 sampleDir = sinTheta * cos(phi) * tangent +
                     sinTheta * sin(phi) * bitangent +
                     cosTheta * normal;
    return normalize(sampleDir);
}

vec3 sdf_normal(vec3 p) {
    const float eps = 0.005;

    float dx = sampleDistance(p + vec3(eps, 0.0, 0.0)) - sampleDistance(p - vec3(eps, 0.0, 0.0));
    float dy = sampleDistance(p + vec3(0.0, eps, 0.0)) - sampleDistance(p - vec3(0.0, eps, 0.0));
    float dz = sampleDistance(p + vec3(0.0, 0.0, eps)) - sampleDistance(p - vec3(0.0, 0.0, eps));
    return normalize(vec3(dx, dy, dz));
}

float sdf_sphere(vec3 p, float r) {
    return length(p) - r;
}

vec3 sphere_normal(vec3 p) {
    const float eps = 0.005;

    float dx = sdf_sphere(p + vec3(eps, 0.0, 0.0), 1.0) - sdf_sphere(p - vec3(eps, 0.0, 0.0), 1.0);
    float dy = sdf_sphere(p + vec3(0.0, eps, 0.0), 1.0) - sdf_sphere(p - vec3(0.0, eps, 0.0), 1.0);
    float dz = sdf_sphere(p + vec3(0.0, 0.0, eps), 1.0) - sdf_sphere(p - vec3(0.0, 0.0, eps), 1.0);
    return normalize(vec3(dx, dy, dz));
}

float weaking(vec3 p, vec3 n) {
    vec3 w_i = normalize(light_pos - p);
    float diff = dot(w_i, n);

    return diff;
}

float sdf_dist(vec3 pos, float t, inout int iter, vec3 ray_origin, vec3 ray_direction, float far) {
    float step_size = cam_block.near;
    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        //float dist = sdf_sphere(p, 1.0);
        float dist = texture(u_volume_tex, get_tex_coord(p)).r;

        if(dist < cam_block.near * cam_block.near) {
            return t;
        }

        if(t > far) {
            break;
        }

        //t += dist;
        t += min(dist * 0.2, step_size);
        iter++;
    }

    return -1.0;
}

float raymarching(vec3 pos, float t_i, float t_f, vec3 ray_origin, vec3 ray_direction) {
    float t = t_i;
    int iter = 0;
    t = sdf_dist(pos, t, iter, ray_origin, ray_direction, min(cam_block.far, t_f));

    return t;
}

float computeDFAO(vec3 pos, vec3 normal) {
    float occlusion = 0.0;
    const int NUM_SAMPLES = 16;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        vec3 sampleDir = sampleHemisphere(normal, i, NUM_SAMPLES);
        int iter = 0;
        float dist = sdf_dist(pos + normal * 0.01, 0.0, iter, pos + normal * 0.01, sampleDir, 1.0);
        if (dist > 0.0) {
            occlusion += 1.0 / (1.0 + dist * dist * 4.0);
        }
    }

    occlusion /= float(NUM_SAMPLES);
    return 1.0 - occlusion;
}

vec4 render(vec2 uv) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    float t0 = 0.0;
    float t1 = cam_block.far;
    bool hit = intersectBox(ray_origin, ray_direction, t0, t1);
    vec3 color = vec3(0.0);

    if (hit) {
        vec3 center = (u_aabb_min + u_aabb_max) * 0.5;
        float t = raymarching(center, t0, t1, ray_origin, ray_direction);

        if(t != -1.0) {
            vec3 p = ray_origin + t * ray_direction;
            float ao = computeDFAO(p, sdf_normal(p));
            color = texture(u_texture, v_position / u_resolution).rgb * ao;
        }
    }

    return vec4(color, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    fragColor = render(uv);
}
