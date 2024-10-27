#name Screen-Space Ambient Occlusion
#renderQueue Lighting
#shaderModule vertex
#version 450

layout(location = 0) in vec2 vertexPosition;

layout(location = 0) out vec2 fragmentTexCoord;
layout(location = 1) out vec2 scaledFragmentTexCoord;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

void main() {
	gl_Position = vec4(vertexPosition, 0.0, 1.0);
	fragmentTexCoord = ((vertexPosition * 0.5f) + vec2(0.5f));
	scaledFragmentTexCoord = fragmentTexCoord * ubo.renderScale; 
}
#endShaderModule
#shaderModule fragment
#version 450

layout(location = 0) in vec2 fragmentTexCoord;
layout(location = 1) in vec2 scaledFragmentTexCoord;
layout(location = 0) out float outColor;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

const int kernelSize = 64;
layout(set = 1, binding = 0) uniform SSAOBufferObject {
	vec4 kernels[kernelSize];
	float radius;
	float bias;
} ssaoUbo;

layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 3) uniform sampler2D gbuffer2;
layout(set = 1, binding = 1) uniform sampler2D ssaoNoise;

vec3 ViewNormal(vec3 inNorm) {
	return mat3(ubo.view) * normalize(inNorm);
}

vec4 ComputeClipSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS = vec4(positionNDC * 2.0 - 1.0, deviceDepth, 1.0);

	return positionCS;
}

vec3 ComputeViewSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS  = ComputeClipSpacePosition(positionNDC, deviceDepth);
	vec4 hpositionWS = inverse(ubo.proj) * positionCS;
	return hpositionWS.xyz / hpositionWS.w;
}

void main() {
	ivec2 noiseScale = ivec2(ubo.renderResolution.x, ubo.renderResolution.y) / 4;

	float depthFromTexture = texture(depthTexture, scaledFragmentTexCoord).x;
	// float m34 = ubo.proj[2][3];
	// float m33 = ubo.proj[2][2];
	// float near = m34 / (m33 - 1);
	// float far = m34 / (m33 + 1);
	// float projectionA = far / (far - near);
	// float projectionB = (-far * near) / (far - near);
	vec3 position = ComputeViewSpacePosition(fragmentTexCoord, depthFromTexture).rgb;
	vec3 normal = ViewNormal(texture(gbuffer2, scaledFragmentTexCoord).rgb);
	vec2 noise = texture(ssaoNoise, fragmentTexCoord * noiseScale).rg;
	vec3 randomVec  = vec3(noise, 0); 

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;

	float radius = ssaoUbo.radius;
	float bias = ssaoUbo.bias;

	for(int i = 0; i < kernelSize; i++) {
		vec3 sampleKernel = TBN * ssaoUbo.kernels[i].xyz;
		sampleKernel = position + sampleKernel * radius;

		vec4 offset = vec4(sampleKernel, 1.0);
		offset      = ubo.proj * offset;			// from view to clip-space
		offset.xyz /= offset.w;						// perspective divide
		offset.xyz  = offset.xyz * 0.5 + vec3(0.5);	// transform to range 0.0 - 1.0
		
		float sampleDepth = texture(depthTexture, offset.xy * ubo.renderScale).x;
		float sampleDepthLinear = ComputeViewSpacePosition(offset.xy, sampleDepth).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepthLinear));
		occlusion += (sampleDepthLinear >= sampleKernel.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	outColor = 1.0 - (occlusion / kernelSize);
}
#endShaderModule
