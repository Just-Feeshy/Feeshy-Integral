#version 410 core

out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    float far;
    float near;
} CamMatrix;

void main() {
    fragColor = vec4(CamMatrix.far / 960.0, CamMatrix.near * 10.0, 0.0, 1.0);
}
