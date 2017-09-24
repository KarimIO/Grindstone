#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
} ubo;

in vec3 vertexPosition;

out vec2 fragTexCoord;
out vec3 viewRay;

void main() {
	fragTexCoord = (vertexPosition.xy + vec2(1,1)) / 2.0;
	gl_Position =  vec4(vertexPosition, 1);

    vec3 positionVS = vec4(gl_Position * ubo.invProj).xyz;
    viewRay = vec3(positionVS.xy / positionVS.z, 1.0f);
}