#line 2

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
const vec3 light_pos = vec3(30.0, 60.0, -60.0);
const float ao_intensity = 0.15; // Ambient Occlusion intensity

#define MAX_STEPS 199
#define NEW_RAYMARCH 1
#define MIN_GROWTH 0.032
#define SDF_FUNC sdfFractal

#define Iterations 8
#define Bailout 100.0
#define Power 12.0

float sdfSphere(vec3 p) {
    return length(p) - 1.0;
}

// Mandelbox Fractal
float sdfFractal(vec3 pos) {
    vec3 z = pos;
    float dr = 2.0;
    float r = 0.0;

    for (int i = 0; i < Iterations; i++) {
        r = length(z);
        if (r > 4.0) break;

        // Convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        dr = pow(r, Power - 1.0) * Power * dr + 1.0;

        // Scale and rotate the point
        float zr = pow(r, Power);
        theta = theta * Power;
        phi = phi * Power;

        // Convert back to cartesian coordinates
        z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += pos;
    }

    return 0.5 * log(r) * r / dr;
}

float ambientOcclusion(vec3 p, vec3 n){
    float step = 0.1;
    float ao = 0.0;
    float dist;
    for(int i = 1; i <= Iterations / 2; i++){
        dist = step * i;
        ao += max((dist - SDF_FUNC(p + n * dist)) / dist, 0.0);
    }
    return (1.0 - ao * ao_intensity);
}

vec3 calcNormal(in vec3 p) {
    const float h = 0.0001;
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*SDF_FUNC( p + k.xyy*h ) +
                      k.yyx*SDF_FUNC( p + k.yyx*h ) +
                      k.yxy*SDF_FUNC( p + k.yxy*h ) +
                      k.xxx*SDF_FUNC( p + k.xxx*h ) );
}

float SDF_distance(float t, inout int iter, vec3 ray_origin, vec3 ray_direction) {
    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = SDF_FUNC(p);

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

    // NEW_RAYMARCH == 2: It's just broken, don't use it
    #if NEW_RAYMARCH == 2
    int iter = 0;
    float t_j = cam_block.far;
    float t_mid = t_j * 0.5;
    bool inside = false;

    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = SDF_FUNC(p);

        if(dist < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        vec3 p_j = ray_origin + t_j * ray_direction;
        float dist_j = SDF_FUNC(p_j);

        vec3 p_mid = ray_origin + t_mid * ray_direction;
        float dist_mid = SDF_FUNC(p_mid);

        if(dist_mid < cam_block.near) {
            inside = true;
            break;
        }

        if(t_mid > cam_block.far) {
            return -1.0;
        }

        if(t_mid < cam_block.near) {
            inside = true;
            break;
        }

        if(abs(dist_j - dist_mid) <= abs(t_j - t_mid)
        && abs(t_j - t_mid) <= dist_mid + dist_j) {
            return -1.0;
        }

        t += dist;
        iter++;
    }

    if(inside) {
        t = SDF_distance(t, iter, ray_origin, ray_direction);
    }

    return -1.0;

    #elif NEW_RAYMARCH == 1

    float t_j = cam_block.far;
    float min_dist = cam_block.far;
    int i = int((MAX_STEPS & 1) == 0);

    #if (MAX_STEPS & 1) == 0
    t = SDF_FUNC(ray_origin + t * ray_direction);

    if(t < cam_block.near) {
        return t;
    }
    #endif

    // {
    //     float t_mid = t_j * 0.5;
    //     vec3 p_mid = ray_origin + t_mid * ray_direction;
    //     float dist_mid = SDF_FUNC(p_mid);

    //     if(dist_mid < cam_block.near) {
    //         int iter = 0;
    //         t = SDF_distance(t_mid, iter, ray_origin, ray_direction);
    //         return t;
    //     }
    // }

    while(i <= (MAX_STEPS >> 1)) {
        vec3 p_i = ray_origin + t * ray_direction;
        float dist_i = SDF_FUNC(p_i);

        if(dist_i < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        if(dist_i - min_dist > MIN_GROWTH) {
            vec3 p_j = ray_origin + t_j * ray_direction;
            float dist_j = SDF_FUNC(p_j);

            if(abs(dist_i - dist_i) <= abs(t_j - t)
            && (dist_i + dist_j) >= abs(t_j - t)) {
                return -1.0;
            }

            t_j -= dist_j;
        }else {
            t += dist_i;
            dist_i = SDF_FUNC(ray_origin + t * ray_direction);

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

vec4 render(vec2 uv) {

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
        vec3 p = ray_origin + t * ray_direction;
        vec3 n = calcNormal(p);
        float light = weaking(p, n);
        color = vec3(1.0, 0.0, 0.0) * light;
        //color = vec3(1.0, 0.0, 0.0);
    }

    return vec4(color, 1.0);
}

void main() {
    vec3 p = vec3(0.0);
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // Apply Raymarching and other techniques
    vec4 color = render(uv);
    fragColor = color;
}
