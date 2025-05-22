#line 2

precision mediump float;

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform sampler2D u_texture;
uniform sampler3D u_volume_tex;
uniform vec3 u_aabb_min;
uniform vec3 u_aabb_max;
uniform vec2 u_resolution;

in vec2 v_position;

bool intersectBox(vec3 ro, vec3 rd, out float t0, out float t1) {
    vec3 inv_dir = 1.0 / rd;
    vec3 t_min = (u_aabb_min - ro) * inv_dir;
    vec3 t_max = (u_aabb_max - ro) * inv_dir;
    vec3 t1s = min(t_min, t_max);
    vec3 t2s = max(t_min, t_max);
    t0 = max(max(t1s.x, t1s.y), t1s.z);
    t1 = min(min(t2s.x, t2s.y), t2s.z);
    return t0 <= t1;
}

vec3 get_tex_coord(vec3 pos) {
    return (pos - u_aabb_min) / (u_aabb_max - u_aabb_min);
}

float sampleDistance(vec3 pos) {
    vec3 tex_coord = get_tex_coord(pos);
    return texture(u_volume_tex, tex_coord).r;
}

bool raymarching(vec3 ray_origin, vec3 ray_direction, out vec3 hit_pos) {
    float t0, t1;

    if (!intersectBox(ray_origin, ray_direction, t0, t1)) {
        return false;
    }

    float t = t0;

    for (int i = 0; i < 100; i++) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = sampleDistance(p);

        if (dist < cam_block.near) {
            hit_pos = p;
            return true;
        }

        if (t > t1) {
            break;
        }

        t += dist;
    }

    return false;
}

float render(vec2 uv) {

    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    vec3 hit_pos;
    if (raymarching(ray_origin, ray_direction, hit_pos)) {
        return 1.0;
    }
    return 0.0;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    fragColor = vec4(render(uv), 0.0, 0.0, 1.0);
}
