#version 330 core

in vec2 fragTexCoord;
out vec4 outval;

uniform sampler2D lighting;

layout(std140) uniform ExposureUBO {
    float exposure;
} exposure_ubo;

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

vec3 hdrGammaTransform(vec3 color) {
    float exposure = exposure_ubo.exposure;
	return linearToSRGB(hdrTransform(color)) * exposure;
}

void main() {
    outval = texture(lighting, fragTexCoord);
    outval = vec4(hdrGammaTransform(outval.rgb), 1);
}