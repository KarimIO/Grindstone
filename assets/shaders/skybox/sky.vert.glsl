#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec2 vertexTexCoord;

out vec3 fragCoord;

void main() {
    // mbo.model messes up everything for some reason
    fragCoord = normalize(vertexPosition);
    gl_Position = ubo.proj_view * vec4(vertexPosition, 1.0);
    // gl_Position.z = gl_Position.w - 0.1;
}