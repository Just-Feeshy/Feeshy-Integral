#line 2

precision mediump float;

out vec4 fragColor;

in vec3 v_texcoord;

uniform samplerCube u_texture;

void main() {
    vec4 texColor = texture(u_texture, v_texcoord);
    fragColor = texColor;
}
