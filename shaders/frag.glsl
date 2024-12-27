#version 410 core

#define MAX_STEPS 100
#define PI 3.14159265
#define TAU (2*PI)

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;
uniform sampler2D u_texture;

float atan2(in float y, in float x) {
    return y > 0.0 ? atan(y, x) + PI : -atan(y, -x);
}

vec2 sphereUV(vec3 p) {
    //p = rotateX(p, PI / 4);
    //p = rotateY(p, 0);
    //p = rotateZ(p, 0);

    float r = length(p);
    float phi = atan2(p.z, p.x);
    return vec2(phi / TAU, acos(p.y / r) / PI);
}

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
    float t_i = 0.0;
    float t_j = MAX_STEPS;

    for(int i = 0; i < (MAX_STEPS >> 1); i++) {
        vec3 p_i = ray_origin + t_i * ray_direction;
        vec3 p_j = ray_origin + t_j * ray_direction;
        float dist_i = sdfSphere(p_i, 1.0);
        float dist_j = sdfSphere(p_j, 1.0);
        float min_dist = min(dist_i, dist_j);

        if(dist_i < cam_block.near) {
            p = p_i;
            return true;
        }

        if(dist_j < cam_block.near) {
            p = p_j;
            return true;
        }

        if((dist_i > cam_block.far || dist_j > cam_block.far)
        || (length(p_j - p_i) * 0.5 < min_dist)) {
            return false;
        }

        t_i += dist_i;
        t_j -= dist_j;
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
        vec2 spTexCoord = sphereUV(normalize(p));
        color = texture(u_texture, spTexCoord);
    }

    fragColor = color;
}
