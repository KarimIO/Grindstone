#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec3 vertexPosition;

void main() {
    gl_Position = ubo.proj_view * mbo.model * vec4(vertexPosition, 1.0);
}