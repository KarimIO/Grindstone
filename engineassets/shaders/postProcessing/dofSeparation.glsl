#name Depth of Field (Separation)
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
	scaledFragmentTexCoord = fragmentTexCoord * ubo.renderScale * 2; 
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

layout(location = 0) in vec2 fragmentTexCoord;
layout(location = 1) in vec2 scaledFragmentTexCoord;

layout(location = 0) out vec4 outNearColor;
layout(location = 1) out vec4 outFarColor;

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
	vec3 litColor = texture(litSceneTexture, scaledFragmentTexCoord).xyz;
	float nearCoc = clamp(1.0 - coc, 0.0, 1.0);
	outNearColor = vec4(litColor, nearCoc);
	float farCoc = clamp(coc, 0.0, 1.0);
	outFarColor = vec4(litColor, farCoc);
}
#endShaderModule
