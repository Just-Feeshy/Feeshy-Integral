#line 2

precision mediump float;

#define MAX_STEPS 199
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

float scene(vec3 p, float r, float off_s, inout vec3 col) {
    float t_2c = cos(u_time * 5.0);
    float t_c = cos(u_time * 10.0);
    float t_s = sin(u_time * 10.0);

    float b_1 = ball(p - vec3(t_2c, t_s, t_c) * 10.0, r);
    float b_2 = ball(p - vec3(t_s, t_2c, t_c) * 10.0, r);
    float b_3 = ball(p - vec3(t_c, t_s, t_2c) * 10.0, r);

    col = vec3(
        off_s - b_1,
        off_s - b_2,
        off_s - b_3
    ) / off_s;

    return smin(smin(b_1, b_2, 2.0), b_3, 2.0);
}

float raymarch(vec3 ray_origin, vec3 ray_direction, inout vec3 col) {
    float offset_size = 10.0;
    float t = 0.0;
    int iter = 0;

    while(iter < MAX_STEPS) {
        vec3 new_col = vec3(0.0);
        vec3 p = ray_origin + t * ray_direction;
        float dist = scene(p, 2.0, offset_size, new_col);
        float second_dist = ground(p);

        if(dist < offset_size) {
            col += exp(-dist * 0.95) * new_col;
        }

        if(dist < cam_block.near) {
            col = new_col;
            return t;
        }

        if(second_dist < cam_block.near) {
            float scale = 0.05;
            vec2 check = floor(p.xz * scale);
            float checker = mod(check.x + check.y, 2.0);

            vec3 darkTile = vec3(0.05);
            vec3 lightTile = vec3(0.2);
            vec3 g_color = mix(darkTile, lightTile, checker);
            float fogDensity = 0.006;
            float fog = 1.0 - exp(-t * fogDensity);
            g_color *= 1.0 - fog;

            col += g_color;
            return t;
        }

        if(t > cam_block.far) {
            break;
        }

        if(dist < second_dist) {
            t += dist; // Adjust step size based on distance
        } else {
            t += second_dist; // Adjust step size based on distance
        }

        iter++;
    }

    return -1.0;
}

vec4 render(vec2 uv) {
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    vec3 color = vec3(0.0);
    float t = raymarch(ray_origin, ray_direction, color);

    if(t != -1.0) {
    }

    vec3 p = ray_origin + t * ray_direction;
    //color  //*= ambientOcclusion(p, n);

    return vec4(color, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Render the scene
    fragColor = render(uv);
}
