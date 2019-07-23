#version 330 core

in vec2 fragTexCoord;
out vec3 outval;

uniform sampler2D lighting;

layout(std140) uniform EffectUBO {
	float vignetteRadius;
	float vignetteSoftness;
	float vignetteStrength;
	float exposure;
	float noiseStrength;
	float time;
} ubo;

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

vec3 vignette(vec3 color) {
	// Get size
	ivec2 resolution = textureSize(lighting, 0);
	vec2 position = (gl_FragCoord.xy / resolution.xy) - vec2(0.5);
	
	// Distance from center
	float len = length(position);
	
	// Use smoothstep to create a smooth vignette
	float vignette = smoothstep(ubo.vignetteRadius, ubo.vignetteRadius - ubo.vignetteSoftness, len);
	vignette = (1 - ubo.vignetteStrength) + (ubo.vignetteStrength * vignette);

	// Mix colors
	return color * vignette;
}

float noise() {
	float noiseStrength = ubo.noiseStrength;
	float time = ubo.time;

    float x = (fragTexCoord.x + 4) * (fragTexCoord.y + 4) * (time * 10);
    float grain = (mod((mod(x, 13.0f) + 1.0f) * (mod(x, 123.0f) + 1.0f), 0.01f) - 0.005f) * noiseStrength;

	return grain;
}

void main() {
	// Main Color
	outval = texture(lighting, fragTexCoord).rgb;

	// Vignetting
	outval = vignette(outval);

	// Add some noise
	outval += vec3(noise());

	// Tonemapped
	float exposure = ubo.exposure;
	outval = hdrTransform(outval * exposure);

	// Gamma Corrected
	outval = linearToSRGB(outval);
}