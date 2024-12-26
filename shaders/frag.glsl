#version 410 core

#define MAX_STEPS 100

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;

float sdfSphere(vec3 p, float radius) {
    return length(p) - radius;
}

bool raymarch(vec2 uv, vec2 ndc, inout vec3 p) {

    // World View Projection
    vec4 clip = vec4(ndc, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    // Summation of Distance
    float t = 0.0;

    for(int i = 0; i < MAX_STEPS; i++) {
        p = ray_origin + ray_direction * t;
        float dist = sdfSphere(p, 1.0);
        t += dist;

        if(dist < cam_block.near) {
            return true;
        }

        if(dist > cam_block.far) {
            return false;
        }
    }

    return false;
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    vec2 ndc = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching
    bool hit_obj = raymarch(uv, ndc, p);
    vec4 color = vec4(0.0);

    if(hit_obj) {
        color = vec4(1.0);
    }


    fragColor = color;
}
