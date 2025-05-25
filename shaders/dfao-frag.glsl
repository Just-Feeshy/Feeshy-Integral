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
        t0 = 0.0;
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
    vec3 tex_coord = clamp(get_tex_coord(pos), vec3(0.001), vec3(0.999));
    return texture(u_volume_tex, tex_coord).r * (length(u_aabb_max - u_aabb_min));
}

// Basically a hemisphere sampling
// I literally learned this from my Calc III class
// So it was pretty easy to implement
vec3 sampleHemisphere(vec3 normal, int i, int total) {
    float phi = TAU * float(i) / float(total); // full circle
    float cos_theta = float(i + 0.5) / float(total);
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    // Local tangent space (TBN) + I used 0.999 for float precision
    vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);

    // Spherical to Cartesian
    vec3 sampleDir = sin_theta * cos(phi) * tangent +
                     sin_theta * sin(phi) * bitangent +
                     cos_theta * normal;
    return normalize(sampleDir);
}

vec3 sdf_normal(vec3 p) {
    const float h = 0.005;
    const vec2 k = vec2(1, -1);
    vec3 n = k.xyy * sampleDistance(p + k.xyy * h) +
             k.yyx * sampleDistance(p + k.yyx * h) +
             k.yxy * sampleDistance(p + k.yxy * h) +
             k.xxx * sampleDistance(p + k.xxx * h);
    return normalize(n);
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

        // Prevent infinite loop from zero/negative distances
        dist = max(dist, 0.02);

        if(t > far) {
            break;
        }

        // t += step_size; // Step size is a constant value
        //t += dist;
        t += min(dist * 0.25, step_size);
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

float compute_AO(vec3 p, vec3 n) {
    float step = cam_block.near * 1.5;
    float ao = 0.0;
    float dist;

    for(int i=1; i<=8; i++) {
        dist = step;
        float weight = exp(-float(i) * 0.5);
        ao += weight * max((dist - sampleDistance(p + n * dist)) / dist, 0.0);
    }

    return 1.0 - ao * 0.3; // Scale the AO value
}

vec4 render(vec2 uv, vec4 tex) {

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
        float t = raymarching(vec3(0.0), t0, t1, ray_origin, ray_direction);
        if(t != -1.0) {
            vec3 p = ray_origin + t * ray_direction;
            color = /*tex.rgb*/ vec3(1.0) * compute_AO(p, sdf_normal(p));
        }

    }

    return vec4(color, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    vec4 tex = texture(u_texture, v_position / u_resolution.xy);

    if (tex.a == 0.0) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    fragColor = render(uv, tex);
}
