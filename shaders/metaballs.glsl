#line 2

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

#define MAX_STEPS 99
#define NEW_RAYMARCH 1
#define MIN_GROWTH 0.032

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

float ground(vec3 p) {
    return p.y + 15.0;
}

float ball(vec3 p, float r) {
    return length(p) - r;
}

float smin(float a, float b, float k) {
    k *= 4.0;
    float x = (b-a)/k;
    float g = (x> 1.0) ? x :
              (x<-1.0) ? 0.0 :
              (x*(2.0+x)+1.0)/4.0;
    return b - k * g;
}

float scene(vec3 p, float r, float off_s) {
    float t_2c = cos(u_time * 5.0);
    float t_c = cos(u_time * 10.0);
    float t_s = sin(u_time * 10.0);

    float b_1 = ball(p - vec3(t_c, t_s, t_2c) * 10.0, r);
    float b_2 = ball(p - vec3(t_s * t_c, t_2c, t_s * t_c) * 10.0, r);
    float b_3 = ball(p - vec3(t_2c, t_s, t_c) * 10.0, r);

    return smin(smin(b_1, b_2, 3.0), b_3, 3.0);
}

float SDL_distance(float t, inout int iter, vec3 ray_origin, vec3 ray_direction, float offset_size) {
    while(iter < MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = scene(p, 2.0, offset_size);

        if(dist < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        t += dist;
        iter++;
    }
}

float raymarch(vec3 ray_origin, vec3 ray_direction) {
    float offset_size = 10.0;
    float t = 0.0;
    int iter = 0;

#if NEW_RAYMARCH == 1

    float t_j = cam_block.far;
    float min_dist = cam_block.far;
    int i = int((MAX_STEPS & 1) == 0);

#if (MAX_STEPS & 1) == 0
    {
        t = scene(ray_origin + t * ray_direction, 2.0, offset_size);

        if(t < cam_block.near) {
            return t;
        }
    }
#endif

    while(i <= (MAX_STEPS >> 1)) {
        vec3 p_i = ray_origin + t * ray_direction;
        float dist_i = scene(p_i, 2.0, offset_size);

        if(dist_i < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        if(dist_i - min_dist > MIN_GROWTH) {
            vec3 p_j = ray_origin + t_j * ray_direction;
            float dist_j = scene(p_j, 2.0, offset_size);

            if(abs(dist_i - dist_j) <= abs(t_j - t)
            && (dist_i + dist_j) >= abs(t_j - t)) {
                return -1.0;
            }

            t_j -= dist_j;
        }else {
            t += dist_i;
            dist_i = scene(ray_origin + t * ray_direction, 2.0, offset_size);

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

#else

    t = SDL_distance(t, iter, ray_origin, ray_direction, offset_size);

#endif

    return t;
}

vec4 render(vec2 uv) {
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    vec3 color = vec3(0.0);
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);
    float t = raymarch(ray_origin, ray_direction);

    if(t != -1.0) {
        color = vec3(1.0, 0.0, 0.0);
    }

    return vec4(color, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    fragColor = render(uv);
}
