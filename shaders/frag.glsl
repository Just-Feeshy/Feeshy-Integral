#version 410 core

#define PI 3.14159265
#define SCENE 2

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

#if SCENE == 1

#define TAU (2*PI)
#define PLANET_RADIUS 2.0

#define RING_RADIUS_2 6.0
#define RING_RADIUS_1 11.0
#define PLANE_TILT 0.1

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


#endif

#if SCENE == 2

#define MAX_STEPS 99
#define NEW_RAYMARCH 1

// Mandelbox Fractal
float sdfFractal(vec3 p) {
    p = p + vec3(
        0.2 * sin(8.0 * u_time + p.y * PI),
        0.2 * cos(4.0 * u_time + p.z * PI),
        0.2 * sin(8.0 * u_time + p.x * PI) * cos(4.0 * u_time + p.y * PI)
    ) + vec3(0.0, 16.0, -32.0);

    vec2 q = vec2(length(p.xz)-8.0,p.y);
    return length(q)-6.0;
}


vec3 calcNormal(in vec3 p) {
    const float h = 0.0001;
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*sdfFractal( p + k.xyy*h ) +
                      k.yyx*sdfFractal( p + k.yyx*h ) +
                      k.yxy*sdfFractal( p + k.yxy*h ) +
                      k.xxx*sdfFractal( p + k.xxx*h ) );
}

float SDF_distance(float t, inout int iter, vec3 ray_origin, vec3 ray_direction) {
    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = sdfFractal(p);

        if(dist < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            break;
        }

        t += dist;
        iter++;
    }

    return -1.0;
}

float raymarch(vec3 ray_origin, vec3 ray_direction) {
    float t = 0.0;

    #if NEW_RAYMARCH == 1
    #define MIN_GROWTH 0.0076

    float t_j = cam_block.far;
    float min_dist = cam_block.far;
    int i = int((MAX_STEPS & 1) == 0);

    #if (MAX_STEPS & 1) == 0
    t = sdfFractal(ray_origin + t * ray_direction);

    if(t < cam_block.near) {
        return t;
    }
    #endif

    while(i <= (MAX_STEPS >> 1)) {
        vec3 p_i = ray_origin + t * ray_direction;
        float dist_i = sdfFractal(p_i);

        if(dist_i < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        if(dist_i - min_dist > MIN_GROWTH) {
            vec3 p_j = ray_origin + t_j * ray_direction;
            float dist_j = sdfFractal(p_j);

            if((dist_i + dist_j) >= abs(t_j - t)) {
                return -1.0;
            }

            t_j -= dist_j;
        }else {
            t += dist_i;
            dist_i = sdfFractal(ray_origin + t * ray_direction);

            if(dist_i < cam_block.near) {
                return t;
            }

            if(t > cam_block.far) {
                return -1.0;
            }
        }

        min_dist = min(min_dist, dist_i);
        t += dist_i;
        i++;
    }

    return -1.0;

    #else

    int iter = 0;
    t = SDF_distance(t, iter, ray_origin, ray_direction);

    #endif

    return t;
}

#endif

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

vec4 render(vec2 uv, vec3 p) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    #if SCENE == 1

    bool hit = false;
    vec2 sp = sphere(4.5, ray_origin, ray_direction, hit);
    vec3 color = stars(ray_direction);

    // Sphere Intersection
    vec3 p_1 = ray_origin + sp.x * ray_direction;

    if(sp.x >= cam_block.near) {
        vec2 spTexCoord = sphereUV(normalize(p_1 - c));
        vec3 normal_sphere = normalize(p_1 - c);

        color = texture(u_texture0, spTexCoord).rgb * weaking(p_1, normal_sphere);
    }


    // Raymarching

    float ringTrace = iPlane(ray_origin, ray_direction, vec4(PLANE_TILT, 1.0, 0.0, 0.0));
    float maxRing = 0.0;
    float ringCol = 0.0;

    if(ringTrace > cam_block.near && ringTrace < cam_block.far) {
        vec3 p_ring = (ray_origin + ringTrace * ray_direction);
        float angle = atan(p_ring.y, p_ring.z);
        float p_ring_len = length((p_ring - c).xz);

        //float ringCol = mix(texture(u_texture1, p_ring.xz).x, 1.0, smoothstep(80.0, 100.0, ringTrace));
        float ringTexture = mix(0.5, 1.0, noise(p_ring_len * 15.0));
        ringCol = mix(0.25, 1.0, clamp(length(p_ring-c) - RING_RADIUS_2 * PLANET_RADIUS, 0.0, 1.0));
        ringCol *= mix(0.45, 1.0, clamp(length(p_ring-c)-7.0 * PLANET_RADIUS, 0.0, 1.0));
        ringCol *= smoothstep(0.0, 0.4, mix(0.0, 0.875, clamp(max(length(p_ring-c)-9.3 * PLANET_RADIUS, -length(p_ring-c)+9.1 * PLANET_RADIUS), 0.0, 1.0))) + 0.125;
        ringCol *= mix(0.5, 1.0, clamp(-length(p_ring-c)+10.5 * PLANET_RADIUS, 0.0, 1.0));

        if(sp.x >= cam_block.near && ringTrace > sp.x) {
            return vec4(color, 1.0);
        }

        if(p_ring_len > RING_RADIUS_2 * PLANET_RADIUS && p_ring_len < RING_RADIUS_1 * PLANET_RADIUS) {
            float minRing = ringCol * ringTexture;
            color = mix(color, vec3(minRing), min(minRing * 1.5, 1.0));
        }else {
            ringCol = 0.0;
        }

    }

    #endif

    #if SCENE == 2

    float t = raymarch(ray_origin, ray_direction);
    vec3 color = vec3(0.0);

    if(t != -1.0) {
        color = vec3(1.0, 0.0, 0.0) * weaking(ray_origin + t * ray_direction, calcNormal(ray_origin + t * ray_direction));
        //color = vec3(1.0, 0.0, 0.0);
    }

    #endif

    return vec4(color, 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv, p);
    fragColor = color;
}
