#name Tonemap Post-Processing
#renderQueue Lighting
#shaderModule vertex
#version 450

layout(location = 0) in vec2 vertexPosition;

layout(location = 0) out vec2 fragmentTexCoord;
layout(location = 1) out vec2 scaledFragmentTexCoord;

layout(binding = 1) uniform sampler2D litSceneTexture;

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

layout(binding = 1) uniform sampler2D litSceneTexture;
layout(binding = 2) uniform sampler2D bloomTexture;
layout(binding = 3) uniform sampler2D depthTexture;

layout(std140, binding = 4) uniform PostProcessUbo {
	vec4 vignetteColor;
	float vignetteRadius;
	float vignetteSoftness;
	float grainAmount;
	float grainPixelSize;
	vec2 chromaticDistortionRedOffset;
	vec2 chromaticDistortionGreenOffset;
	vec2 chromaticDistortionBlueOffset;
	float paniniDistortionStrength;
	bool isAnimated;
} postProcessingUbo;

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

vec3 applyVignette(vec3 color, vec2 screenOffset) {
	float radius = postProcessingUbo.vignetteRadius;
	float distanceFromCenter = length(screenOffset);
	float vignette = smoothstep(radius, radius - postProcessingUbo.vignetteSoftness, distanceFromCenter);

	return mix(postProcessingUbo.vignetteColor.rgb, color, clamp(vignette, 0, 1));
}

vec3 applyChromaticAbberation(sampler2D colorTexture, vec2 texCoord, vec2 direction) {
	vec3 color = vec3(0.0f);
	color.r  = texture(colorTexture, texCoord + (direction * postProcessingUbo.chromaticDistortionRedOffset)).r;
	color.g  = texture(colorTexture, texCoord + (direction * postProcessingUbo.chromaticDistortionGreenOffset)).g;
	color.b = texture(colorTexture, texCoord + (direction * postProcessingUbo.chromaticDistortionBlueOffset)).b;

	return color;
}

vec3 applyGrain(vec3 originalColor, vec2 resolution, vec2 texCoord) {
	float time = postProcessingUbo.isAnimated
		? ubo.time * 8000.0f
		: 0.0f;
	vec2 noiseCoords = ivec2(texCoord * resolution / postProcessingUbo.grainPixelSize) / resolution;
	float noise = (fract(sin(dot(noiseCoords + time, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;
	return originalColor + noise * postProcessingUbo.grainAmount;
}

const float Pi = 3.14159265359;
const float Pi05 = Pi * 0.5;

float Pow2(float val) {
	return val*val;
}

/* http://tksharpless.net/vedutismo/Pannini/panini.pdf */
vec3 paniniProjection(vec2 texCoord, float fov) {
	float distortionStrength = postProcessingUbo.paniniDistortionStrength;
	float distortionStrength2 = distortionStrength*distortionStrength;

	{
		float fo = Pi05 - fov * 0.5;

		float f = cos(fo) / sin(fo) * 2.0;
		float f2 = f*f;

		float b = (sqrt(max(0.0, Pow2(distortionStrength+distortionStrength2)*(f2+f2*f2))) - (distortionStrength*f+f)) / (distortionStrength2+distortionStrength2*f2-1.0);

		texCoord *= b;
	}
	
	float h = texCoord.x;
	float v = texCoord.y;
	
	float h2 = h*h;
	
	float k = h2/Pow2(distortionStrength+1.0);
	float k2 = k*k;
	
	float discriminant = max(0.0, k2 * distortionStrength2 - (k + 1.0) * (k * distortionStrength2 - 1.0));
	
	float cosPhi = (-k * distortionStrength + sqrt(discriminant))/(k+1.0);
	float S = (distortionStrength+1.0)/(distortionStrength+cosPhi);
	float tanTheta = v/S;
	
	float sinPhi = sqrt(max(0.0, 1.0 - cosPhi*cosPhi));

	if(texCoord.x < 0.0) {
		sinPhi *= -1.0;
	}

	float s = inversesqrt(1.0 + tanTheta*tanTheta);
	
	return vec3(sinPhi, tanTheta, cosPhi) * s;
}

void main() {
	float fov = 1.5707963f / 2.0f;
	vec2 texCoordToSample = fragmentTexCoord * 2.0f - vec2(1.0f);
	texCoordToSample = paniniProjection(texCoordToSample, fov).xy / 2.0f + vec2(0.5f);

	vec2 resolution = ubo.renderResolution;
	float aspectRatio = resolution.x / resolution.y;

	if (texCoordToSample.x <= 0.0f || texCoordToSample.y <= 0.0f || texCoordToSample.x > 1.0f || texCoordToSample.y > 1.0f) {
		outColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		return;
	}
	
	vec2 screenOffset = texCoordToSample;
	screenOffset = (screenOffset * 2.0f) - vec2(1.0f);
	screenOffset = (aspectRatio > 1.0f)
		? vec2(screenOffset.x / aspectRatio, screenOffset.y)
		: vec2(screenOffset.x, screenOffset.y / aspectRatio);

	vec3 sceneColor = vec3(0.0f);

	// Get color directly if not using chromatic abberation.
	// vec3 sceneColor = texture(litSceneTexture, distortedUV).rgb;
	sceneColor = applyChromaticAbberation(litSceneTexture, texCoordToSample * ubo.renderScale, screenOffset * screenOffset);

	sceneColor = sceneColor + texture(bloomTexture, texCoordToSample * ubo.renderScale * 2).rgb;
	sceneColor = applyVignette(sceneColor, screenOffset);
	sceneColor = applyGrain(sceneColor, resolution, fragmentTexCoord);
	sceneColor = hdrTransform(sceneColor);
	sceneColor = linearToSRGB(sceneColor);

	outColor = vec4(sceneColor, 1);
}
#endShaderModule
