#name Directional Light
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

layout(binding = 1) uniform sampler2D gbuffer0;
layout(binding = 2) uniform sampler2D gbuffer1;
layout(binding = 3) uniform sampler2D gbuffer2;
layout(binding = 4) uniform sampler2D gbuffer3;
layout(binding = 5) uniform sampler2D gbuffer4;

layout(set = 1, binding = 0) uniform LightUbo {
	mat4 shadowMatrix;
	vec3 color;
	float sourceRadius;
	vec3 direction;
	float intensity;
	float shadowResolution;
} light;

layout(set = 1, binding = 1) uniform sampler2D shadowMap;

const float pi = 3.14159f;

// Schlick
vec3 Light_F(in vec3 f0, in float f90, in float VH) {
	return f0 + (f90-f0) * pow(1-VH, 5.0f);
}

// GGX
float Light_D(in float alphaSqr, in float NH) {
	float denom = NH * NH * (alphaSqr - 1) + 1;

	return alphaSqr / (pi * denom * denom);
}

float Light_V( in float NL, in float NV, in float alphaSqr ) {
	float Lambda_GGXV = NL * sqrt (( - NV * alphaSqr + NV ) * NV + alphaSqr );
	float Lambda_GGXL = NV * sqrt (( - NL * alphaSqr + NL ) * NL + alphaSqr );

	return 0.25f / max( Lambda_GGXV + Lambda_GGXL, 0.0001f );
}

float Diff_Disney(float NdotV, float NdotL, float LdotH, float linearRoughness) {
	float energyBias = 0 * (1-linearRoughness) + 0.5*linearRoughness;
	float energyFactor = 1.0 * (1-linearRoughness) + linearRoughness / 1.51;
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
	vec3 f0 = vec3(1.0f , 1.0f , 1.0f);
	float lightScatter = Light_F( f0 , fd90 , NdotL ) .r;
	float viewScatter = Light_F(f0 , fd90 , NdotV ).r;

	return lightScatter * viewScatter * energyFactor;
}

vec3 BSDF(float NV, float NL, float LH, float NH, float alpha, vec3 SpecularColor) {
	vec3 f0 = 0.32*SpecularColor*SpecularColor;
	float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);
	
	vec3 F = Light_F(f0, f90, LH);
	float D = Light_D(alpha, NH);
	float Vis = Light_V(NL, NV, alpha);

	return (F * D * Vis);
}

vec3 LightDirCalc(
	in vec3 albedo,
	in vec3 position,
	in vec3 specularTexture,
	in float roughness,
	in vec3 normal,
	in vec3 lightDir,
	in vec3 lightColor,
	in vec3 eyePos
) {
	float lightDistance	= length(lightDir);

	vec3 eyeDir		= normalize(eyePos - position);
	vec3 eyeReflect = reflect(-eyeDir, normal);
	
	lightDir		= -normalize(lightDir);

	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;

	float NL = clamp(dot(normal, lightDir), 0, 1);

	vec3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(normal, eyeDir)) + 0.00001;
	float NH = clamp(dot(normal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);

	vec3 specular = BSDF(NV, NL, LH, NH, alpha, specularTexture.rgb);
	vec3 diffuse = albedo.rgb * vec3(Diff_Disney(NV, NL, LH, roughness)) / pi;

	vec3 BSDFValue = diffuse + specular;
	return vec3(NL * BSDFValue * lightColor.xyz);
}

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float ShadowRandom(vec4 seed4) {
	float dotProduct = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dotProduct) * 43758.5453);
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
	
	float depth = texture(gbuffer0, scaledFragmentTexCoord).r;
	vec3 position = ComputeWorldSpacePosition(fragmentTexCoord, depth);
	vec3 diffuse = texture(gbuffer1, scaledFragmentTexCoord).rgb;
	vec3 normal = texture(gbuffer2, scaledFragmentTexCoord).rgb;
	vec3 specular = gbuffer3Value.rgb;
	float roughness = gbuffer3Value.a;

	vec3 lightPow = light.color * light.intensity;
	
	vec3 litValues = LightDirCalc(
		diffuse,
		position,
		specular,
		roughness,
		normal,
		light.direction,
		lightPow,
		ubo.eyePos
	);

	float nl = dot(light.direction, normal);
	vec4 lightSpacePos = light.shadowMatrix * vec4(position, 1);
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	float pixelDepth = projCoords.z;
	
	const vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
	const float pcfKernelSize = 1.0f;
	float shadow = 0.0f;

	for (int i = 0; i < 16; ++i) {
		float randomAngle = ShadowRandom(vec4(floor(position.xyz * 1000.0f), i));
		vec2 rotation = vec2(cos(randomAngle) * 0.5f + 0.5f, sin(randomAngle) * 0.5f + 0.5f);
		vec2 offset = vec2(
			rotation.x * poissonDisk[i].x - rotation.y * poissonDisk[i].y,
			rotation.y * poissonDisk[i].x + rotation.x * poissonDisk[i].y
		);
 
		float pcfDepth = texture(shadowMap, projCoords.xy + offset * texelSize * 4.0f).r;
		bool isInLight = pcfDepth + 0.05f >= pixelDepth;
		shadow += isInLight ? 1.0f : 0.0f;
	}

	shadow = shadow / 16;

	bool isInMap = projCoords.z >= 0 && projCoords.z <= 1;
	float isInLightMap = (isInMap) ? shadow : 0.0f;

	outColor = vec4(vec3(isInLightMap) * litValues, 1);
}
#endShaderModule
