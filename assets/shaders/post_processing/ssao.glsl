#version 330 core

in vec2 fragTexCoord;
layout(location = 0) out vec4 SSAOout;

layout(std140) uniform DefferedUBO {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

const int kernelSize = 32;
layout(std140) uniform SSAOBufferObject {
    vec3 kernels[kernelSize];
    float radius;
    float bias;
} ssao_ubo;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform sampler2D ssao_noise;

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

vec3 ViewNormal(vec3 inNorm) {
    return (transpose(ubo.invView) * normalize(vec4(inNorm, 1.0))).rgb;
}

void main() {
    ivec2 noiseScale = textureSize(gbuffer1, 0) / 4;
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 position = ViewPosFromDepth(Dist, fragTexCoord).xyz;
	vec3 normal = texture(gbuffer1, fragTexCoord).xyz;
    normal = ViewNormal(normal);
    vec2 noise = texture(ssao_noise, fragTexCoord * noiseScale).rg;
    vec3 randomVec  = vec3(2 * noise - 1, 0); 

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    float radius = ssao_ubo.radius;
    float bias = ssao_ubo.bias;

    for(int i = 0; i < kernelSize; i++) {
        vec3 sampleKernel = TBN * ssao_ubo.kernels[i].xyz;
        sampleKernel = position + sampleKernel * radius;

        vec4 offset = vec4(sampleKernel, 1.0);
        offset      = inverse(ubo.invProj) * offset;    // from view to clip-space
        offset.xy /= offset.w;               // perspective divide
        offset.xy  = offset.xy * 0.5 + vec2(0.5); // transform to range 0.0 - 1.0

	    float Dist = texture(gbuffer3, offset.xy).r;
        float sampleDepth = ViewPosFromDepth(Dist, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= sampleKernel.z ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    SSAOout = vec4(vec3(occlusion), 1);
}