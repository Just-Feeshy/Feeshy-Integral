#line 2

precision mediump float;

layout(location = 0) in vec2 a_position;

uniform vec2 u_resolution;

out vec2 v_position;

void main() {
    vec2 rect = 2.0 * (a_position / u_resolution) - vec2(1.0);
    gl_Position = vec4(rect, 0.0, 1.0);
    v_position = a_position;
}
