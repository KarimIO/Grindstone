#version 330 core

in vec2 vertexPosition;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

out vec2 ctrlTexCoord;

void main() {
    gl_Position = vec4(vertexPosition.x, 0.0, vertexPosition.y, 1.0);
    gl_Position = gl_Position;
    ctrlTexCoord = (vertexPosition)/240.0f;
}