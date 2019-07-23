#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec3 vertexPosition;

out vec3 fragPosition;

void main() {
    //  mbo.model messes up everything for some reason
    fragPosition = (vec4(100.0f * vertexPosition, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    gl_Position = gl_Position.xyww;
}