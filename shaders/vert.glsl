#version 410 core

layout(location = 0) in vec2 a_position;

uniform vec2 u_resolution;

void main() {
    vec2 rect = 2.0 * (a_position / u_resolution) - vec2(1.0);
    gl_Position = vec4(rect, 0.0, 1.0);
}
