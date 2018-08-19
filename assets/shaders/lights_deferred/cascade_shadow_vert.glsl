#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

const int NUM_CASCADES = 3;

in vec3 vertexPosition;

out vec2 fragTexCoord;
out vec3 viewRay;

layout(std140) uniform Light {
	mat4 shadow_mat[NUM_CASCADES];
	float cascade_ends[NUM_CASCADES];
	vec3 direction;
	float sourceRadius;
	vec3 color;
	float power;
	bool shadow;
} light;

out vec4 LightSpacePos[NUM_CASCADES];
out float ClipSpacePosZ;

void main() {
	fragTexCoord = (vertexPosition.xy + vec2(1,1)) / 2.0;
	gl_Position = vec4(vertexPosition, 1);
    
    for (int i = 0 ; i < NUM_CASCADES ; i++) {
        LightSpacePos[i] = light.shadow_mat[i] * gl_Position;
    }

    ClipSpacePosZ = gl_Position.z;

    vec3 positionVS = vec4(gl_Position * ubo.invProj).xyz;
    viewRay = vec3(positionVS.xy / positionVS.z, 1.0f);
}