#name Image-Based Lighting
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
layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 2) uniform sampler2D gbuffer1;
layout(binding = 3) uniform sampler2D gbuffer2;
layout(binding = 4) uniform sampler2D gbuffer3;
layout(binding = 5) uniform sampler2D gbuffer4;
layout(set = 1, binding = 0) uniform sampler2D ssao;
layout(set = 1, binding = 1) uniform sampler2D brdfLUT;
layout(set = 2, binding = 0) uniform samplerCube specularMap;

const float pi = 3.14159f;

vec3 Light_F(in vec3 f0, in float f90, in float VH) {
	return f0 + (f90-f0) * pow(1-VH, 5.0f);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	uv.y = -uv.y;
	return uv;
}

vec3 sphericalHarmonics[] =  {
	// First Band
	vec3(2.58676, 2.730808, 3.152812),
	// Second Band
	vec3(-0.431493, -0.665128, -0.969124),
	vec3(-0.353886, 0.048348, 0.672755),
	vec3(-0.604269, -0.88623, -1.298684),
	// Third Band
	vec3(0.320121, 0.422942, 0.541783),
	vec3(-0.137435, -0.168666, -0.229637),
	vec3(-0.052101, -0.149999, -0.232127),
	vec3(-0.117312, -0.167151, -0.265015),
	vec3(-0.090028, -0.021071, 0.08956)
};

vec3 GetIrradiance(vec3 normal) {
	float x = normal.z;
	float y = normal.y;
	float z = normal.x;

	float c[] = {
		0.282095,
		0.488603,
		1.092548,
		0.315392,
		0.546274
	};
	
	vec3 result = (
		sphericalHarmonics[0] * c[0] +

		sphericalHarmonics[1] * c[1] * x +
		sphericalHarmonics[2] * c[1] * y +
		sphericalHarmonics[3] * c[1] * z +

		sphericalHarmonics[4] * c[2] * z * x +
		sphericalHarmonics[5] * c[2] * y * z +
		sphericalHarmonics[6] * c[2] * y * x +
		sphericalHarmonics[7] * c[3] * (3.0 * z * z - 1.0) +
		sphericalHarmonics[8] * c[4] * (x*x - y*y)
	);

	return max(result / pi, vec3(0.0));
}

vec4 ComputeClipSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS = vec4(positionNDC * 2.0 - 1.0, deviceDepth, 1.0);

	return positionCS;
}

vec3 ComputeWorldSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS  = ComputeClipSpacePosition(positionNDC, deviceDepth);
	vec4 hpositionWS = inverse(ubo.proj * ubo.view) * positionCS;
	return hpositionWS.xyz / hpositionWS.w;
}

void main() {
	vec4 gbuffer3Value = texture(gbuffer3, scaledFragmentTexCoord);

	float depthFromTexture = texture(depthTexture, scaledFragmentTexCoord).r;

	vec3 position = ComputeWorldSpacePosition(fragmentTexCoord, depthFromTexture);
	vec3 albedo = texture(gbuffer1, scaledFragmentTexCoord).rgb;
	vec3 normal = texture(gbuffer2, scaledFragmentTexCoord).rgb;
	vec3 specularInput = gbuffer3Value.rgb;
	float roughness = gbuffer3Value.a * gbuffer3Value.a;
	float ao = texture(ssao, scaledFragmentTexCoord).r;
	
	vec3 eyeDir = normalize(ubo.eyePos.xyz - position);
	vec3 reflectRay = 2 * dot(eyeDir, normal) * normal - eyeDir;
	
	float NV = max(dot(normal, eyeDir), 0.0);
	
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 f0 = 0.32 * specularInput * specularInput;
	float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);
	vec3 F = Light_F(f0, f90, NV);
	vec3 prefilteredColor = textureLod(specularMap, reflectRay, roughness * MAX_REFLECTION_LOD).rgb;   
	vec2 envBRDF  = texture(brdfLUT, vec2(NV, roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	vec3 irradiance = GetIrradiance(normal);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	vec3 diffuse    = irradiance * albedo;
	vec3 ambient    = (kD * diffuse + specular) * ao;

	outColor = vec4(ambient, 1);
}
#endShaderModule
