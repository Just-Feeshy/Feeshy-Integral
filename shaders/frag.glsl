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


// Credits to nimitz (https://www.shadertoy.com/user/nimitz)
// Original Source: https://www.shadertoy.com/view/fl2Bzd
vec4 hash43x(vec3 p) {
    uvec3 x = uvec3(ivec3(p));
    x = 1103515245U*((x.xyz >> 1U)^(x.yzx));
    uint h = 1103515245U*((x.x^x.z)^(x.y>>3U));
    uvec4 rz = uvec4(h, h*16807U, h*48271U, h*69621U);
    return vec4((rz >> 1) & uvec4(0x7fffffffU))/float(0x7fffffff);
}

// Credits to nimitz (https://www.shadertoy.com/user/nimitz)
// Original Source: https://www.shadertoy.com/view/fl2Bzd
vec3 stars(vec3 p)
{
    vec3 col = vec3(0);
    float rad = .087*u_resolution.y;
    float dens = 0.15;
    float id = 0.;
    float rz = 0.;
    float z = 1.;

    for (float i = 0.; i < 5.; i++) {
        p *= mat3(0.86564, -0.28535, 0.41140, 0.50033, 0.46255, -0.73193, 0.01856, 0.83942, 0.54317);
        vec3 q = abs(p);
        vec3 p2 = p/max(q.x, max(q.y,q.z));
        p2 *= rad;
        vec3 ip = floor(p2 + 1e-5);
        vec3 fp = fract(p2 + 1e-5);
        vec4 rand = hash43x(ip*283.1);
        vec3 q2 = abs(p2);
        vec3 pl = 1.0- step(max(q2.x, max(q2.y, q2.z)), q2);
        vec3 pp = fp - ((rand.xyz-0.5)*.6 + 0.5)*pl;
        float pr = length(ip) - rad;
        if (rand.w > (dens - dens*pr*0.035)) pp += 1e6;

        float d = dot(pp, pp);
        d /= pow(fract(rand.w*172.1), 32.) + .25;
        float bri = dot(rand.xyz*(1.-pl),vec3(1));
        id = fract(rand.w*101.);
        col += bri*z*.00009/pow(d + 0.025, 3.0)*(mix(vec3(1.0,0.45,0.1),vec3(0.75,0.85,1.), id)*0.6+0.4);

        rad = floor(rad);
        dens *= 0.5;
        p = p.yxz;
    }

    return col;
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
            return vec2(t0, t1 - t0);
        }
    }

    return vec2(-1.0, 1.0);
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
        return texture(u_texture, spTexCoord);
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
