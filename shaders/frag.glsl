#version 410 core

#define MAX_STEPS 100
#define PI 3.14159265
#define TAU (2*PI)
#define NEW_RAYMARCH 1

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;
uniform float u_time;
uniform int u_quality;

const vec3 c = vec3(0.0, 0.0, 3.0);
const vec3 light_pos = vec3(3.0, 60.0, -60.0);

// I don't want to use a mat3x3 for this
vec3 rotateX(vec3 p, float angle) {
    float cosT = cos(angle);
    float sinT = sin(angle);

    return vec3(
        p.x,
        p.y * cosT - p.z * sinT,
        p.y * sinT + p.z * cosT
    );
}

// I don't want to use a mat3x3 for this
vec3 rotateY(vec3 p, float angle) {
    float cosT = cos(angle);
    float sinT = sin(angle);

    return vec3(
        p.x * cosT + p.z * sinT,
        p.y,
        p.z * cosT - p.x * sinT
    );
}

float sdfSphere(vec3 p, float radius) {
    return length(p - c) - radius;
}

float raymarching(vec3 ray_origin, vec3 ray_direction) {
    float t = 0.0;

    #if NEW_RAYMARCH
    for (int i = 0; i < (MAX_STEPS / 2); i++) {
        vec3 p = ray_origin + ray_direction * t;
        float dist = sdfSphere(p, 2.0);
        //vec3 p_j = ray_origin + ray_direction * 

        if (dist < cam_block.near) {
            return t;
        }

        if (t > cam_block.far || dist > cam_block.far) {
            break;
        }

        t += dist;
    }
    #else
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ray_origin + ray_direction * t;
        float dist = sdfSphere(p, 2.0);

        if (dist < cam_block.near) {
            return t;
        }

        if (t > cam_block.far) {
            break;
        }

        t += dist;
    }
    #endif

    return -1.0;
}

vec4 render(vec2 uv, vec3 p) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    float dist = raymarching(ray_origin, ray_direction);

    if (dist >= 0.0) {
        vec3 color = vec3(1.0, 0.0, 0.0);
        return vec4(color, 1.0);
    }

    return vec4(0.0, 0.0, 0.0, 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv, p);
    fragColor = color;
}
