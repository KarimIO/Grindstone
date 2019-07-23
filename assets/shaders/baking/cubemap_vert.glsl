#version 330 core
layout (location = 0) in vec3 vertexPosition;

out vec3 fragPosition;

layout(std140) uniform ConvolutionBufferObject {
    mat4 projview;
    float roughness;
} ubo;

void main() {
    fragPosition = vec3(vertexPosition); // ubo.projview * 
    gl_Position =  ubo.projview * vec4(vertexPosition * -5.0f, 1.0);
}