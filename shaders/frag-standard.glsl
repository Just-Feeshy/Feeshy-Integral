#line 2

precision mediump float;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform vec2 u_resolution;

in vec2 v_position;

void main() {
    fragColor = texture(u_texture, v_position / u_resolution);
}
