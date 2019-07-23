#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eyepos;
    float time;
    mat4 invProj;
    mat4 invView;
	vec4 resolution;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec3 vertexPosition;

out vec3 fragCoord;

void main() {
    //  mbo.model messes up everything for some reason
    fragCoord = (vec4(100.0f * vertexPosition, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragCoord, 1.0);
    gl_Position = gl_Position.xyww;
}