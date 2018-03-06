#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

in vec3 vertexPosition;

out vec3 fragCoord;

void main() {
    gl_Position = vec4(((gl_VertexID & 1) << 2) - 1, (gl_VertexID & 2) * 2 - 1, 1.0, 1.0);
    //vec4(vertexPosition, 1);
    fragCoord = mat3(ubo.invView) * (ubo.invProj * gl_Position).xyz;
    // vec4(gl_Position * ubo.invProj).xyz;
}