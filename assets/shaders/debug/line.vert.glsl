#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec4 vertexColor;

out vec3 fragPosition;
out vec4 fragColor;

void main() {
    gl_Position = ubo.proj_view * vec4(vertexPosition, 1.0);
    fragColor = vertexColor;
}