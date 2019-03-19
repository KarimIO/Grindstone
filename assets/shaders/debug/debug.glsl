#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
    vec4 resolution;
	float time;
} ubo;

layout(std140) uniform Debug {
    uint mode;
} debug;

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

vec3 ViewNormal(vec3 inNorm) {
    return (transpose(ubo.invView) * normalize(vec4(inNorm, 1.0))).rgb;
}

void main() {
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec4 ViewPos = ViewPosFromDepth(Dist, fragTexCoord);
	vec3 Position = WorldPosFromViewPos(ViewPos);
	vec3 Normal = texture(gbuffer1, fragTexCoord).rgb;
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;
	vec3 Specular = texture(gbuffer2, fragTexCoord).rgb;
	float Roughness = texture(gbuffer2, fragTexCoord).a;

    if (debug.mode == 1u) {
        Dist /= 5.0f;
	    outColor = vec4(Dist, Dist, Dist, 1);
    }
    else if (debug.mode == 2u) {
        outColor = vec4(Normal, 1);
    }
    else if (debug.mode == 3u) {
        outColor = vec4(ViewNormal(Normal), 1);
    }
    else if (debug.mode == 4u) {
        outColor = vec4(Albedo, 1);
    }
    else if (debug.mode == 5u) {
        outColor = vec4(Specular, 1);
    }
    else if (debug.mode == 6u) {
        outColor = vec4(Roughness, Roughness, Roughness, 1);
    }
    else if (debug.mode == 7u) {
        outColor = vec4(Position, 1);
    }
    else {
        outColor = vec4(fragTexCoord, 0, 1);
    }
}