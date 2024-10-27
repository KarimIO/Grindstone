#name Bloom
#renderQueue Lighting
#shaderModule compute
#version 450

const float epsilon = 1.0e-4;

layout(std140, binding = 0) uniform ControlUniformBuffer {
	vec4 reciprocalImgSizes; // Consists of  vec2 in; vec2 out;
	vec4 thresholdFilter;
	int stage;
	float levelOfDetail;
	float filterRadius;
} ubo;

layout(binding = 1, rgba32f) restrict writeonly uniform image2D outImage;
layout(binding = 2) uniform sampler2D inImage1;
layout(binding = 3) uniform sampler2D inImage2;

const int bloomStagePrefilter	= 0;
const int bloomStageDownsample	= 1;
const int bloomStageUpsample	= 2;
const int bloomStageApply		= 3;

vec3 Upsample(sampler2D srcTexture, vec2 texCoord, vec2 texelSize, float radius) {
	vec2 offset = texelSize * radius;

	vec3 result = texture(srcTexture, texCoord).rgb * 4.0f;

	result += texture(srcTexture, texCoord + vec2(-offset.x, 0)).rgb * 2.0;
	result += texture(srcTexture, texCoord + vec2( offset.x, 0)).rgb * 2.0;
	result += texture(srcTexture, texCoord + vec2(0, -offset.y)).rgb * 2.0;
	result += texture(srcTexture, texCoord + vec2(0,  offset.y)).rgb * 2.0;

	result += texture(srcTexture, texCoord + vec2(-offset.x,-offset.y)).rgb;
	result += texture(srcTexture, texCoord + vec2(-offset.x, offset.y)).rgb;
	result += texture(srcTexture, texCoord + vec2( offset.x,-offset.y)).rgb;
	result += texture(srcTexture, texCoord + vec2( offset.x, offset.y)).rgb;

	return result * (1.0f / 16.0f);
}

vec3 PowVec3(vec3 v, float p) {
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float RGBToLuminance(vec3 col) {
    return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(vec3 col) {
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

vec3 TransformKaris(vec3 col) {
	return col * KarisAverage(col);
}

vec3 Downsample(sampler2D srcTexture, vec2 texCoord, vec2 texelSize, bool isFirstDownsample) {
	texCoord += texelSize / 2.0f;

	vec3 a = texture(srcTexture, texCoord + texelSize * vec2(-2.0f,-2.0f)).rgb;
	vec3 b = texture(srcTexture, texCoord + texelSize * vec2( 0.0f,-2.0f)).rgb;
	vec3 c = texture(srcTexture, texCoord + texelSize * vec2( 2.0f,-2.0f)).rgb;
	
	vec3 d = texture(srcTexture, texCoord + texelSize * vec2(-2.0f, 0.0f)).rgb;
	vec3 e = texture(srcTexture, texCoord + texelSize * vec2( 0.0f, 0.0f)).rgb;
	vec3 f = texture(srcTexture, texCoord + texelSize * vec2( 2.0f, 0.0f)).rgb;

	vec3 g = texture(srcTexture, texCoord + texelSize * vec2(-2.0f, 2.0f)).rgb;
	vec3 h = texture(srcTexture, texCoord + texelSize * vec2( 0.0f, 2.0f)).rgb;
	vec3 i = texture(srcTexture, texCoord + texelSize * vec2( 2.0f, 2.0f)).rgb;
	
	vec3 j = texture(srcTexture, texCoord + texelSize * vec2(-1.0f,-1.0f)).rgb;
	vec3 k = texture(srcTexture, texCoord + texelSize * vec2(-1.0f, 1.0f)).rgb;
	vec3 l = texture(srcTexture, texCoord + texelSize * vec2( 1.0f, 1.0f)).rgb;
	vec3 m = texture(srcTexture, texCoord + texelSize * vec2( 1.0f,-1.0f)).rgb;

    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
	
	if (isFirstDownsample) {
		return	  TransformKaris((e) * 0.125f)
				+ TransformKaris((a + c + g + i) * 0.03125f)
				+ TransformKaris((b + d + f + h) * 0.0625f)
				+ TransformKaris((j + k + l + m) * 0.125f);
	}
	else {
		return ((e) * 0.125f)
				+ ((a + c + g + i) * 0.03125f)
				+ ((b + d + f + h) * 0.0625f)
				+ ((j + k + l + m) * 0.125f);
	}
}

vec4 Prefilter(vec4 color) {
	float brightness = max(color.r, max(color.g, color.b));
	float soft = brightness - ubo.thresholdFilter.y;
	soft = clamp(soft, 0.0, ubo.thresholdFilter.z);
	soft = soft * soft * ubo.thresholdFilter.w;
	
	return color * max(soft, brightness - ubo.thresholdFilter.x) / max(brightness, epsilon);
}

layout(local_size_x = 4, local_size_y = 4) in;
void main() {
	ivec2 invocID = ivec2(gl_GlobalInvocationID);
	vec2 texCoords = vec2(invocID.x * ubo.reciprocalImgSizes.z, invocID.y * ubo.reciprocalImgSizes.w);
	
	vec4 color = vec4(0.0f);

	if (ubo.stage == bloomStagePrefilter) {
		color.rgb = Downsample(inImage1, texCoords, ubo.reciprocalImgSizes.zw, true);
		color = max(Prefilter(color), 0.0f);
		color.a = 1.0f;
	}
	else if (ubo.stage == bloomStageDownsample) {
		color.rgb = Downsample(inImage1, texCoords, ubo.reciprocalImgSizes.zw, false);
	}
	else if (ubo.stage == bloomStageUpsample) {
		vec3 upsampledTexture = Upsample(inImage1, texCoords, ubo.reciprocalImgSizes.xy, ubo.filterRadius);
		
		vec3 existing = texture(inImage2, texCoords).rgb;
		color.rgb = existing + upsampledTexture;
	}
	else if (ubo.stage == bloomStageApply) {
		color.rgb = Upsample(inImage1, texCoords, ubo.reciprocalImgSizes.xy, ubo.filterRadius);
	}

	imageStore(outImage, invocID, color);
}
#endShaderModule
