#version 410 core

#define MAX_STEPS 100
#define PI 3.14159265
#define TAU (2*PI)
#define PLANET_RADIUS 2.0
#define NEW_RAYMARCH 0

#define RING_RADIUS_2 6.0
#define RING_RADIUS_1 10.0

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;
uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
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

// Function taken from "The Book of Shaders"
// Credits to Patricio Gonzalez Vivo & Jen Lowe
// Original Source: https://thebookofshaders.com/10/
float hash( const in float n ) {
    return fract(sin(n)*43758.5453123);
}

float noise( const in  float p ) {
    float i = floor(p);
    float f = fract(p);
    return mix( hash( i + 0. ), hash( i + 1. ), f*f*f);
}

// Intersection of a plane
float iPlane( in vec3 ro, in vec3 rd, in vec4 pla ) {
    return (-pla.w - dot(pla.xyz,ro)) / dot( pla.xyz, rd );
}

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

vec2 sphereUV(vec3 p) {
    //p = rotateX(p, PI / 4);
    p = rotateY(p, u_time);
    //p = rotateZ(p, 0);

    float r = length(p);
    float phi = atan2(p.z, p.x);
    return vec2(phi / TAU, acos(p.y / r) / PI);
}

// At the end of the day, it's just a quadratic formula
vec2 quadratic(float a, float b, float c, inout bool hit) {
    float d = b * b - 4.0 * a * c;
    if(d < 0.0) {
        hit = false;
        return vec2(-1.0, 1.0);
    }

    hit = true;
    return vec2(
        (-b - sqrt(d)) / (2.0 * a),
        (-b + sqrt(d)) / (2.0 * a)
    );
}

// Most basic raytracing example on how raytracing actually works
// TO WRITE: How this works and the basics of raytracing
vec2 sphere(float r, vec3 rayOrigin, vec3 rayDirection, inout bool hit) {
    vec3 oc = rayOrigin - c;
    const float a = 1.0;

    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - r * r * PLANET_RADIUS * PLANET_RADIUS;
    vec2 disc = quadratic(a, b, c, hit);

    return disc;
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

float sdfSphere(vec3 p, vec3 d, float r) {
    p.z = fract(p.z);
    return length(p - 0.5) - r;
}

float raymarch(vec3 ray_origin, vec3 ray_direction) {
    float t = 0.0;

    #if NEW_RAYMARCH
    #else

    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ray_origin + t * ray_direction;
        //float dist = sdTorus(p - c, vec2(2.0, 0.05));
        float dist = sdfSphere(p, t * ray_direction, 0.25);

        if(dist < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            break;
        }

        t += dist;
    }
    #endif

    return -1.0;
}

vec3 sub_render(vec3 color, vec3 ray_origin, vec3 ray_direction, vec2 sp) {

    // Indivudual rings for the planet
    float t_objs = raymarch(ray_origin, ray_direction);
    if(t_objs > 0.0) {
        if(sp.x >= cam_block.near && t_objs > sp.x) {
            return color;
        }

        // color = vec3(1.0, 0.0, 0.0);
    }

    return color;
}

vec4 render(vec2 uv, vec3 p) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    bool hit = false;
    vec2 sp = sphere(4, ray_origin, ray_direction, hit);
    vec3 color = stars(ray_direction);

    // Sphere Intersection
    vec3 p_1 = ray_origin + sp.x * ray_direction;

    if(sp.x >= cam_block.near) {
        vec2 spTexCoord = sphereUV(normalize(p_1 - c));
        vec3 normal_sphere = normalize(p_1 - c);

        color = texture(u_texture0, spTexCoord).rgb * weaking(p_1, normal_sphere);
    }


    // Raymarching

    //vec2 ringTrace = march_ring(ray_origin, ray_direction);
    float ringTrace = iPlane(ray_origin, ray_direction, vec4(0.1, 1.0, 0.0, 0.0));

    if(ringTrace > cam_block.near && ringTrace < cam_block.far) {
        if(sp.x >= cam_block.near && ringTrace > sp.x) {
            return vec4(sub_render(color, ray_origin, ray_direction, sp), 1.0);
        }

        vec3 p_ring = (ray_origin + ringTrace * ray_direction);
        float angle = atan(p_ring.y, p_ring.z);
        float p_ring_len = length((p_ring - c).xz);

        //float ringCol = mix(texture(u_texture1, p_ring.xz).x, 1.0, smoothstep(80.0, 100.0, ringTrace));
        float ringCol = mix(0.5, 1.0, noise(p_ring_len * 15.0));
        ringCol *= mix(0.25, 1.0, clamp(length(p_ring-c)-6.5 * PLANET_RADIUS, 0.0, 1.0));
        ringCol *= mix(0.45, 1.0, clamp(length(p_ring-c)-7.0 * PLANET_RADIUS, 0.0, 1.0));
        ringCol *= smoothstep(0.0, 0.4, mix(0.0, 0.875, clamp(max(length(p_ring-c)-8.8 * PLANET_RADIUS, -length(p_ring-c)+8.6 * PLANET_RADIUS), 0.0, 1.0))) + 0.125;
        ringCol *= mix(0.5, 1.0, clamp(-length(p_ring-c)+9.5 * PLANET_RADIUS, 0.0, 1.0));

        //color = mix(color, vec3(ringCol), smoothstep(40.0, 50.0, ringTrace.x));
        if(p_ring_len > RING_RADIUS_2 * PLANET_RADIUS && p_ring_len < RING_RADIUS_1 * PLANET_RADIUS) {
            ringCol *= mix(texture(u_texture1, p_ring.xz).x, 1.0, smoothstep(40.0, 50.0, ringTrace));
            ringCol *= mix(0.0, ringCol, smoothstep(20.0, 25.0, ringTrace));
            color = mix(color, vec3(ringCol), min(ringCol * 3.0, 1.0));
        }
    }

    return vec4(sub_render(color, ray_origin, ray_direction, sp), 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv, p);
    fragColor = color;
}
