precision mediump float;

#define PI 3.14159265

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

const vec3 c = vec3(0.0, 0.0, 3.0);
const vec3 light_pos = vec3(3.0, 60.0, -60.0);

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

    float t = raymarch(ray_origin, ray_direction);
    vec3 color = vec3(0.0);

    if(t != -1.0) {
        color = vec3(1.0, 0.0, 0.0) * weaking(ray_origin + t * ray_direction, calcNormal(ray_origin + t * ray_direction));
        //color = vec3(1.0, 0.0, 0.0);
    }

    return vec4(color, 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv, p);
    fragColor = color;
}
