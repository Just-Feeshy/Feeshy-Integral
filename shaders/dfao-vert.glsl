#line 2

precision mediump float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec3 a_tangent;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform mat4 u_model;

out vec2 v_texcoord;

void main() {
    vec4 world_position = vec4(a_position, 1.0);
    world_position.xyz -= cam_block.position;

    mat4 mvp = cam_block.projection * cam_block.view * u_model;
    gl_Position = mvp * world_position;
    v_texcoord = a_texcoord;
}
