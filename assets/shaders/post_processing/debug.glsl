#version 330 core

in vec2 fragTexCoord;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;

out vec4 outColor;

layout(std140) uniform DebugBufferObject {
    uint mode;
} dbo;

void main() {
    outColor = vec4(fragTexCoord, 0, 1);
}