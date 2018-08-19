#version 330 core

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec2 vertexPosition;

uniform mat4 gWorld;

out vec3 WorldPos_CS_in;
out vec2 TexCoord_CS_in;
out vec3 Normal_CS_in;

void main()
{
    WorldPos_CS_in = (mbo.model * vec4(vertexPosition.x, 1.0, vertexPosition.y, 1.0)).xyz;
    TexCoord_CS_in = vertexPosition;
    Normal_CS_in = vec3(0, 1, 0);
} 