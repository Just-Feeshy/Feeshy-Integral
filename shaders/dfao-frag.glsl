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
uniform sampler3D u_texture2;
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

void main() {
    fragColor = texture(u_texture, vec2(v_position / u_resolution));
}
