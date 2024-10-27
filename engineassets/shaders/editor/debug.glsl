#name Debug Shader
#renderQueue Lighting
#shaderModule vertex
#version 450

layout(location = 0) in vec2 vertexPosition;

layout(location = 0) out vec2 fragmentTexCoord;
layout(location = 1) out vec2 scaledFragmentTexCoord;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec3 eyePos;
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

layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec3 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

layout(binding = 1) uniform sampler2D gbufferDepth;
layout(binding = 2) uniform sampler2D gbufferAlbedo;
layout(binding = 3) uniform sampler2D gbufferNormal;
layout(binding = 4) uniform sampler2D gbufferSpecularRoughness;
layout(binding = 5) uniform sampler2D ambientOcclusion;

layout(std140, binding = 6) uniform DebugUbo {
	uint mode;
} debugUbo;

vec3 linearToSRGB(vec3 inColor) {
	vec3 outColor;
	outColor.r = inColor.r <= 0.0031308 ? 12.92 * inColor.r : 1.055 * pow(inColor.r, 1.0/2.4) - 0.055;
	outColor.g = inColor.g <= 0.0031308 ? 12.92 * inColor.g : 1.055 * pow(inColor.g, 1.0/2.4) - 0.055;
	outColor.b = inColor.b <= 0.0031308 ? 12.92 * inColor.b : 1.055 * pow(inColor.b, 1.0/2.4) - 0.055;
	return outColor;
}

vec3 hdrTransform(vec3 color) {
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	return clamp((color*(a*color+b))/(color*(c*color+d)+e), 0, 1);
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

vec3 ComputeWorldSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS  = ComputeClipSpacePosition(positionNDC, deviceDepth);
	vec4 hpositionWS = inverse(ubo.proj * ubo.view) * positionCS;
	return hpositionWS.xyz / hpositionWS.w;
}

vec3 ViewNormal(vec3 inNorm) {
	return mat3(ubo.view) * normalize(inNorm);
}

#define RenderMode_Default 0
#define RenderMode_Position 1
#define RenderMode_PositionMod 2
#define RenderMode_ViewPosition 3
#define RenderMode_ViewPositionMod 4
#define RenderMode_Depth 5
#define RenderMode_DepthMod 6
#define RenderMode_Normal 7
#define RenderMode_ViewNormal 8
#define RenderMode_Albedo 9
#define RenderMode_Specular 10
#define RenderMode_Roughness 11
#define RenderMode_AmbientOcclusion 12

void main() {
	switch (debugUbo.mode) {
		case RenderMode_Position: {
			float depth = texture(gbufferDepth, scaledFragmentTexCoord).r;
			vec3 pos = ComputeWorldSpacePosition(fragmentTexCoord, depth);
			outColor = vec4(pos, 1);
			break;
		}
		case RenderMode_PositionMod: {
			float depth = texture(gbufferDepth, scaledFragmentTexCoord).r;
			vec3 pos = mod(abs(ComputeWorldSpacePosition(fragmentTexCoord, depth)), 1.0f);
			outColor = vec4(pos, 1);
			break;
		}
		case RenderMode_ViewPosition: {
			float depth = texture(gbufferDepth, scaledFragmentTexCoord).r;
			vec3 pos = ComputeViewSpacePosition(fragmentTexCoord, depth);
			outColor = vec4(pos, 1);
			break;
		}
		case RenderMode_ViewPositionMod: {
			float depth = texture(gbufferDepth, scaledFragmentTexCoord).r;
			vec3 pos = mod(abs(ComputeViewSpacePosition(fragmentTexCoord, depth)), 1.0f);
			outColor = vec4(pos, 1);
			break;
		}
		case RenderMode_Depth: {
			float depth = texture(gbufferDepth, scaledFragmentTexCoord).r;
			outColor = vec4(depth, depth, depth, 1);
			break;
		}
		case RenderMode_DepthMod: {
			float depth = mod(ComputeViewSpacePosition(scaledFragmentTexCoord, texture(gbufferDepth, scaledFragmentTexCoord).r).z, 1.0f);
			outColor = vec4(depth, depth, depth, 1);
			break;
		}
		case RenderMode_Normal: {
			vec3 normal = texture(gbufferNormal, scaledFragmentTexCoord).rgb;
			outColor = vec4(normal, 1);
			break;
		}
		case RenderMode_ViewNormal: {
			vec3 normal = ViewNormal(texture(gbufferNormal, scaledFragmentTexCoord).rgb);
			outColor = vec4(normal, 1);
			break;
		}
		case RenderMode_Albedo: {
			vec3 albedo = texture(gbufferAlbedo, scaledFragmentTexCoord).rgb;
			outColor = vec4(albedo, 1);
			break;
		}
		case RenderMode_Specular: {
			vec3 specular = texture(gbufferSpecularRoughness, scaledFragmentTexCoord).rgb;
			outColor = vec4(specular, 1);
			break;
		}
		case RenderMode_Roughness: {
			float roughness = texture(gbufferSpecularRoughness, scaledFragmentTexCoord).a;
			outColor = vec4(roughness, roughness, roughness, 1);
			break;
		}
		case RenderMode_AmbientOcclusion: {
			float ao = texture(ambientOcclusion, scaledFragmentTexCoord).r;
			outColor = vec4(ao, ao, ao, 1);
			break;
		}
		default: {
			outColor = vec4(0, 0, 0, 1);
			break;
		}
	}
}
#endShaderModule
