#line 2

precision mediump float;

out vec4 fragColor;

in vec2 v_texcoord;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform sampler2D u_texture;
uniform mat4 u_model;

void main() {
    vec4 texColor = texture(u_texture, v_texcoord);

    if (texColor.a < 0.01) {
        discard;
    }

    //fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    fragColor = texColor;
}
