#line 2

precision mediump float;

#define PI 3.14159265
#define TAU 6.28318530

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform sampler3D u_volume_tex;
uniform vec3 u_aabb_min;
uniform vec3 u_aabb_max;
uniform vec2 u_resolution;

const vec3 c = vec3(0.0, 0.0, 3.0);
const vec3 light_pos = vec3(3.0, 60.0, -60.0);
const float EPSILON = 0.01;
const float step_size = 0.0; // Step size for ray marching
const float MIN_GROWTH = 0.064; // Minimum growth for ray marching

in vec2 v_position;

#define MAX_STEPS 299
#define NEW_RAYMARCH 1

const float ao_max_iterations = 4; // Maximum iterations for Ambient Occlusion
const float ao_intensity = 0.25; // Ambient Occlusion intensity

// Thank you for "A Minimal Ray-Tracer"
// The original code can be found at:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
bool intersectBox(vec3 ro, vec3 rd, out float t0, out float t1) {
    vec3 bounds[2] = vec3[2](u_aabb_min, u_aabb_max);
    vec3 inv_dir = 1.0 / rd;
    ivec3 sign = ivec3(// Better than using the `step` function
        (inv_dir.x < 0.0),
        (inv_dir.y < 0.0),
        (inv_dir.z < 0.0)
    );

    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    tmin = (bounds[sign.x].x - ro.x) * inv_dir.x;
    tmax = (bounds[1 - sign.x].x - ro.x) * inv_dir.x;
    tymin = (bounds[sign.y].y - ro.y) * inv_dir.y;
    tymax = (bounds[1 - sign.y].y - ro.y) * inv_dir.y;

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin) {
        tmin = tymin;
    }

    if (tymax < tmax) {
        tmax = tymax;
    }

    tzmin = (bounds[sign.z].z - ro.z) * inv_dir.z;
    tzmax = (bounds[1 - sign.z].z - ro.z) * inv_dir.z;

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    if (tzmin > tmin) {
        tmin = tzmin;
    }

    if (tzmax < tmax) {
        tmax = tzmax;
    }

    t0 = tmin;
    t1 = tmax;

    if(t0 < 0.0 && t1 >= 0.0) {
        t0 = 0.0;
    }

    if (t1 < 0.0) {
        return false;
    }

    return true;
}

vec3 get_tex_coord(vec3 pos) {
    return clamp((pos - u_aabb_min) / (u_aabb_max - u_aabb_min), vec3(0.0), vec3(1.0));
}

float sampleDistance(vec3 pos) {
    vec3 tex_coord = get_tex_coord(pos);
    float dist = texture(u_volume_tex, tex_coord).r;
    
    // Debug: Check for invalid values
    if (isnan(dist) || isinf(dist)) {
        return 1000.0; // Large distance for invalid values
    }
    
    return dist;
}

float ambientOcclusion(vec3 p, vec3 n){
    float step = 0.1;
    float ao = 0.0;
    float dist;
    for(int i = 1; i <= ao_max_iterations; i++){
        dist = step * i;
        ao += max((dist - sampleDistance(p + n * dist)) / dist, 0.0);
    }

    return (1.0 - ao * ao_intensity);
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

vec3 sdf_normal(vec3 p) {
    const float h = 0.005;
    const vec2 k = vec2(1, -1);
    vec3 n = k.xyy * sampleDistance(p + k.xyy * h) +
             k.yyx * sampleDistance(p + k.yyx * h) +
             k.yxy * sampleDistance(p + k.yxy * h) +
             k.xxx * sampleDistance(p + k.xxx * h);
    return normalize(n);
}

float get_accomodated_distance(float dist) {
    // This is a simple function to accommodate the distance
    // based on the current step size and the distance.
    // It can be adjusted based on the requirements.
    return min((dist / length(u_aabb_max - u_aabb_min)), EPSILON);
}

float sdf_dist(vec3 pos, float t, inout int iter, vec3 ray_origin, vec3 ray_direction, float far) {
    float maxStep = 0.5 * length(u_aabb_max - u_aabb_min) / 128.0;
    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = sampleDistance(p);

        if(dist < EPSILON) {
            return t;
        }

        // t += clamp(dist, EPSILON, maxStep)
        t += get_accomodated_distance(dist);

        if(t > far) {
            break;
        }
        iter++;
    }

    return -1.0;
}

float raymarching(vec3 pos, float t_i, float t_f, vec3 ray_origin, vec3 ray_direction) {
    float t = t_i;

    #if NEW_RAYMARCH == 1

    float t_j = t_f;
    float min_dist = cam_block.far;
    float step_size = cam_block.near;
    int i = int((MAX_STEPS & 1) == 0);
    int first_cases = 0;

    #if (MAX_STEPS & 1) == 0
    t = sampleDistance(ray_origin + t * ray_direction);

    if(t < cam_block.near * cam_block.near) {
        return t;
    }
    #endif

    while(i <= (MAX_STEPS >> 1)) {
        vec3 p_i = ray_origin + t * ray_direction;
        float dist_i = sampleDistance(p_i);

        if(dist_i < EPSILON) {
            return t;
        }

        if(t > t_f) {
            return -1.0;
        }

        if(dist_i - min_dist > MIN_GROWTH) {
            vec3 p_j = ray_origin + t_j * ray_direction;
            float dist_j = sampleDistance(p_j);

            if(abs(dist_i - dist_j) <= abs(t_j - t)
            && (dist_i + dist_j) >= abs(t_j - t)) {
                return -1.0;
            }

            t_j -= dist_j;
        } else { // Though this is suppose to be 'j' in the paper, it is 'i' in the code
            t += get_accomodated_distance(dist_i);
            float dist_j = sampleDistance(ray_origin + t * ray_direction);

            if(dist_j < cam_block.near * cam_block.near) {
                return t;
            }

            if(t > t_f) {
                return -1.0;
            }
        }

        min_dist = min(min_dist, dist_i);
        t += get_accomodated_distance(dist_i);
        i++;
    }

    return -1.0;

    #else

    int iter = 0;
    t = sdf_dist(pos, t, iter, ray_origin, ray_direction, min(cam_block.far, t_f));

    #endif

    return t;
}

vec4 render(vec2 uv) {
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    float t0 = 0.0;
    float t1 = cam_block.far;
    bool hit = intersectBox(ray_origin, ray_direction, t0, t1);
    vec3 color = vec3(0.0);

    if (hit && t1 > t0) {
        // Debug: Show box intersection in green
        color = vec3(0.0, 1.0, 0.0);
        
        // Debug: Sample texture at entry point to check for corruption
        vec3 entry_point = ray_origin + t0 * ray_direction;
        vec3 tex_coord = get_tex_coord(entry_point);
        float sample_dist = texture(u_volume_tex, tex_coord).r;
        
        // Debug visualization modes
        if (uv.x < -0.8) {
            // Show raw texture sampling
            color = vec3(sample_dist * 0.1); // Scale for visibility
        } else if (uv.x < -0.6) {
            // Show texture coordinates
            color = tex_coord;
        } else if (uv.x < -0.4) {
            // Show AABB bounds test
            color = vec3(
                step(u_aabb_min.x, entry_point.x) * step(entry_point.x, u_aabb_max.x),
                step(u_aabb_min.y, entry_point.y) * step(entry_point.y, u_aabb_max.y),
                step(u_aabb_min.z, entry_point.z) * step(entry_point.z, u_aabb_max.z)
            );
        } else {
            // Normal rendering
            float t = raymarching(vec3(0.0), t0, t1, ray_origin, ray_direction);
            if(t != -1.0) {
                vec3 p = ray_origin + t * ray_direction;
                vec3 n = sdf_normal(p);
                color = vec3(1.0, 0.0, 0.0) * ambientOcclusion(p, n);
            }
        }
    }

    return vec4(color, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    // fragColor = texture(u_volume_tex, vec3(0.0, 0.0, 0.0));
    fragColor = render(uv);
}
