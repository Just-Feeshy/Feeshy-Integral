#line 2

precision mediump float;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform vec2 u_resolution;

in vec2 v_position;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

void main() {
    vec4 texColor = texture(u_texture, v_position / u_resolution);
    fragColor = texColor;
}
