#name Depth of Field (Combine)
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
	scaledFragmentTexCoord = fragmentTexCoord * ubo.renderScale / 2.0;
}
#endShaderModule
#shaderModule fragment
#version 450

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D depthTexture;
layout(set = 1, binding = 1) uniform sampler2D litSceneTexture;
layout(set = 2, binding = 0) uniform sampler2D nearTexture;
layout(set = 2, binding = 1) uniform sampler2D farTexture;

layout(location = 0) in vec2 fragmentTexCoord;
layout(location = 1) in vec2 scaledFragmentTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	float depthFromTexture = texture(depthTexture, scaledFragmentTexCoord).x;
	float m34 = ubo.proj[2].w;
	float m33 = ubo.proj[2].z;
	float near = m34 / (m33 - 1.0);
	float far = m34 / (m33 + 1.0);
	float ndc = (depthFromTexture * 2.0) - 1.0;
	float linearDepth = ((2.0 * near) * far) / ((far + near) - (ndc * (far - near)));
	float apertureF = 2.0;
	float focalLength = 0.0500000007450580596923828125;
	float apertureSize = focalLength / apertureF;
	float focalDistance = 8.0;
	float sensorHeight = 0.0024000001139938831329345703125;
	float coc = ((-apertureSize) * (focalLength * (focalDistance - linearDepth))) / (linearDepth * (focalDistance - focalLength));
	coc /= sensorHeight;
	vec4 unblurredColor = texture(litSceneTexture, scaledFragmentTexCoord);
	outColor = vec4(unblurredColor.rgb, 1);
	return;
	vec4 nearColor = texture(nearTexture, scaledFragmentTexCoord * 2);
	vec3 farColor = texture(farTexture, scaledFragmentTexCoord * 2).xyz;
	vec3 focusedFarMix = mix(unblurredColor.xyz, farColor, vec3(clamp(coc, 0.0, 1.0)));
	outColor = vec4(mix(focusedFarMix, nearColor.xyz, vec3(nearColor.w)), unblurredColor.w);
}
#endShaderModule
