#version 330 core

in vec2 fragTexCoord;
uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;

out vec4 outColor;

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

layout(std140) uniform DebugBufferObject {
    uint mode;
} dbo;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition.xyz /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

void main() {
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 position = WorldPosFromDepth(Dist, fragTexCoord).xyz;
	vec3 normal = texture(gbuffer1, fragTexCoord).xyz;
    vec3 albedo = texture(gbuffer0, fragTexCoord).rgb;
    vec4 specular = texture(gbuffer2, fragTexCoord);
    
    switch(dbo.mode) {
    default:
        outColor = vec4(position, 1);
        break;
    case 2u:
        outColor = vec4(normal, 1);
        break;
    case 3u:
        outColor = vec4(albedo, 1);
        break;
    case 4u:
        outColor = vec4(specular.rgb, 1);
        break;
    case 5u:
        outColor = vec4(specular.aaa, 1);
        break;
    case 6u:
        outColor = vec4(Dist, Dist, Dist, 1);
        break;
    }
}