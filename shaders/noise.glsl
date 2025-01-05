#version 410 core

#define MAX_STEPS 1000

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;
uniform sampler2D u_texture1;

const vec3 c = vec3(0.0, 0.0, 3.0);

float sdfCube(vec3 p) {
    vec3 q = abs(p) - 2.0;
    return max(max(q.x, q.y), q.z);
}

vec2 raymarch(vec3 ray_origin, vec3 ray_direction) {
    vec2 trace = vec2(-1.0);
    float t = 0.0;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ray_origin + ray_direction * t;
        float dist = sdfCube(p - c);

        if(dist < cam_block.near) {
            trace = vec2(t, dist);
            break;
        }

        if(dist > cam_block.far) {
            break;
        }

        t += dist;
    }

    return trace;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);
    vec2 render = raymarch(ray_origin, ray_direction);

    if(render.x >= 0.0) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
