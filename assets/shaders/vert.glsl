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

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

void main() {
    // mbo.model messes up everything for some reason
    gl_Position = ubo.proj_view * vec4(vertexPosition, 1.0);
    fragPosition = (mbo.model * vec4(vertexPosition, 1.0)).xyz;
    fragNormal = normalize((mbo.model * vec4(vertexNormal, 1.0)).xyz);
    fragTangent =  normalize((mbo.model * vec4(vertexTangent, 1.0)).xyz);
    fragTexCoord = vec2(vertexTexCoord.x, -vertexTexCoord.y);
}