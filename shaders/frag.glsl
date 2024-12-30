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
uniform sampler2D u_texture;
uniform float u_time;

const vec3 c = vec3(0.0, 0.0, 3.0);
const vec3 light_pos = vec3(3.0, 6.0, -6.0);


// Credits to nimitz (https://www.shadertoy.com/user/nimitz)
// Original Source: https://www.shadertoy.com/view/XsyGWV
vec3 nmzHash33(vec3 q) {
  uvec3 p = uvec3(ivec3(q));
  p = p * uvec3(374761393U, 1103515245U, 668265263U) + p.zxy + p.yzx;
  p = p.yzx * (p.zxy ^ (p >> 3U));
  return vec3(p ^ (p >> 16U)) * (1.0 / vec3(0xffffffffU));
}

// Credits to nimitz (https://www.shadertoy.com/user/nimitz)
// Original Source: https://www.shadertoy.com/view/XsyGWV
vec3 stars(in vec3 p) {
  vec3 c = vec3(0.);
  float res = u_resolution.x * 0.8;

  for(float i = 0.; i < 5.; i++) {
    vec3 q = fract(p * (.15 * res)) - 0.5;
    vec3 id = floor(p * (.15 * res));
    vec2 rn = nmzHash33(id).xy;
    float c2 = 1. - smoothstep(0., .6, length(q));
    c2 *= step(rn.x, .0005 + i * 0.002);
    c += c2 * (mix(vec3(1.0, 0.49, 0.1), vec3(0.75, 0.9, 1.), rn.y) * 0.25 + 0.75);
    p *= 1.4;
  }
  return c * c;
}
float atan2(in float y, in float x) {
    return y > 0.0 ? atan(y, x) + PI : -atan(y, -x);
}

vec3 rotateY(vec3 p, float angle) {
    float cosT = cos(angle);
    float sinT = sin(angle);

    return vec3(
        p.x * cosT + p.z * sinT,
        p.y,
        p.z * cosT - p.x * sinT
    );
}

vec2 sphereUV(vec3 p) {
    //p = rotateX(p, PI / 4);
    p = rotateY(p, u_time);
    //p = rotateZ(p, 0);

    float r = length(p);
    float phi = atan2(p.z, p.x);
    return vec2(phi / TAU, acos(p.y / r) / PI);
}

// Most basic raytracing example on how raytracing actually works
// TO WRITE: How this works and the basics of raytracing
vec2 sphere(float r, vec3 rayOrigin, vec3 rayDirection) {
    vec3 oc = rayOrigin - c;
    const float a = 1.0;

    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - r * r;
    float discriminant = b * b - 4.0 * a * c;

    if(discriminant > 0.0) {
        float s = sqrt(discriminant);
        float t0 = max(cam_block.near, (-b - s) / (2.0 * a));
        float t1 = (-b + s) / (2.0 * a);

        if(t1 >= cam_block.near) {
            return vec2(t0, t1);
        }
    }

    return vec2(-1.0, 1.0);
}

// Most basic writing for lighting
// TO WRITE: How this works and the basics of lighting
// @param n - The normal of the surface
// @var w_i - The negative direction of the incoming light
// (aka. the direction from the light to the surface)
float weaking(vec3 p, vec3 n) {
    vec3 w_i = normalize(light_pos - p);
    float diff = dot(w_i, n);

    return diff;
}

vec4 render(vec2 uv, inout vec3 p) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    // Raymarching
/*
#if NEW_RAYMARCH
    float t = 0.0;

    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 p_i = ray_origin + t * ray_direction;
        vec3 p_j = ray_origin + (MAX_STEPS - 1.0 - t) * ray_direction;
        float dist_i = sdfSphere(p_i, 1.0);
        float dist_j = sdfSphere(p_j, 1.0);
        float min_dist = min(dist_i, dist_j);

        if(min_dist > cam_block.far
        || (length(p_j - p_i) * 0.5 <= min_dist)) {
            break;
        }

        if(dist_i < cam_block.near) {
            p = p_i;
            return true;
        }

        t += dist_i;
    }
#else
    float t = 0.0;

    for(int i = 0; i < MAX_STEPS; i++) {
        p = ray_origin + t * ray_direction;
        float dist = sdfSphere(p, 1.0);

        if(dist < cam_block.near) {
            return true;
        }

        if(dist > cam_block.far) {
            break;
        }

        t += dist;
    }
#endif
*/

    vec2 sp = sphere(1.0, ray_origin, ray_direction);
    if(sp.x >= cam_block.near) {
        p = ray_origin + sp.x * ray_direction;
        vec2 spTexCoord = sphereUV(normalize(p - c));
        vec3 normal_sphere = normalize(p - c);

        return texture(u_texture, spTexCoord) * weaking(p, normal_sphere);
    }

    return vec4(stars(ray_direction), 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv, p);
    fragColor = color;
}
