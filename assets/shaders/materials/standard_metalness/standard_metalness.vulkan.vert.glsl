#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(binding = 1) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexTexCoord;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec3 fragTangent;
layout (location = 3) out vec2 fragTexCoord;

void main() {
    // mbo.model messes up everything for some reason
    fragPosition = (mbo.model * vec4(vertexPosition, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragNormal = normalize((mbo.model * vec4(vertexNormal, 0.0)).xyz);
    fragTangent =  normalize((mbo.model * vec4(vertexTangent, 0.0)).xyz);
    fragTexCoord = vec2(vertexTexCoord.x, -vertexTexCoord.y);
}