#line 2

precision mediump float;

uniform sampler2D u_texture;

in vec2 v_texcoord;

out vec4 fragColor;

void main() {
    vec4 tex_color = texture(u_texture, v_texcoord);
    fragColor = tex_color;
}
