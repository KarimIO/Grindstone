#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec2 vertexPosition;

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

uniform sampler2D heightmap;

void main() {
    vec2 tex = (vertexPosition)/240.0f;
    float heightmap = texture(heightmap, tex).r * 20.0f;

    // mbo.model messes up everything for some reason
    fragPosition = (mbo.model * vec4(vertexPosition.x, heightmap, vertexPosition.y, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragNormal = vec3(0,1,0);
    fragTangent =  vec3(0,1,0);
    fragTexCoord = tex;
}