#line 2

precision mediump float;

layout(location = 0) in vec3 a_position;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

out vec3 v_texcoord;

void main() {
    vec4 world_position = vec4(a_position, 1.0);
    mat4 vp = cam_block.projection * cam_block.view;

    gl_Position = vp * world_position;
    v_texcoord = a_position;
}
