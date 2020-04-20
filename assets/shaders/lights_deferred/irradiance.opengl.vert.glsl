#version 330 core

in vec3 vertexPosition;

layout(std140) uniform DefferedUBO {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

layout(std140) uniform SphericalHarmonicsBuffer {
	mat4 pvm;
    vec3 sh0[9];
    vec3 sh1[9];
} spherical_harmonic_list;

out vec3 viewRay;

void main() {
	gl_Position =  inverse(ubo.invProj) * inverse(ubo.invView) * vec4(10 * vertexPosition, 1);

    vec3 positionVS = vec4(gl_Position * ubo.invProj).xyz;
    viewRay = vec3(positionVS.xy / positionVS.z, 1.0f);
}