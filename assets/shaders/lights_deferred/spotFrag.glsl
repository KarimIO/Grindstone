#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
    vec4 resolution;
} ubo;

layout(std140) uniform Light {
	mat4 shadow_mat;
	vec3 position;
	float attenuationRadius;
	vec3 color;
	float power;
	vec3 direction;
	float innerAngle;
	float outerAngle;
} light;

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform sampler2D shadow_map;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

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

	return 0.25f / ( Lambda_GGXV + Lambda_GGXL );
}

float Diff_Disney( float NdotV, float NdotL, float LdotH ,float linearRoughness ) {
	float energyBias = 0*(1-linearRoughness) + 0.5*linearRoughness;
	float energyFactor = 1.0*(1-linearRoughness) + linearRoughness / 1.51;
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness ;
	vec3 f0 = vec3(1.0f , 1.0f , 1.0f);
	float lightScatter = Light_F( f0 , fd90 , NdotL ) .r;
	float viewScatter = Light_F(f0 , fd90 , NdotV ).r;

	return lightScatter * viewScatter * energyFactor ;
}

vec3 BSDF(float NV, float NL, float LH, float NH, float alpha, vec3 SpecularColor) {
	vec3 f0 = 0.32*SpecularColor*SpecularColor;
	float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);
	
	vec3 F = Light_F(f0, f90, LH);
	float D = Light_D(alpha, NH);
	float Vis = Light_V(NL, NV, alpha);

	return (F * D * Vis);
}

vec3 LightPointCalc(in vec3 Albedo, in vec3 WorldPos, in vec4 Specular, in vec3 Normal, in vec3 lightPos, in float lightRadius, in vec3 lightColor, in vec3 eyePos) {
	vec3 lightDir	= WorldPos - lightPos;
	float Distance	= length(lightDir);

	vec3 eyeDir		= normalize(eyePos - WorldPos);
	vec3 eyeReflect = reflect(-eyeDir, Normal);
	
	lightDir		= -normalize(lightDir);
	
	float Roughness = Specular.a;
	float alpha = Roughness * Roughness;
	float alphaSqr = alpha*alpha;

	float NL = clamp(dot(Normal, lightDir), 0, 1);
	
	float Dist2 = Distance * Distance;
	float lrad2 = lightRadius * lightRadius;
	float factor = Dist2 / lrad2;
	float Attenuation = clamp(1 - factor * factor, 0, 1); 
	Attenuation = Attenuation*Attenuation/(Dist2+0.001);

	vec3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(Normal, eyeDir)) + 0.00001;
	float NH = clamp(dot(Normal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);
	
	//vec3 reflColor =  texture(texRefl, eyeReflect).rgb;

	vec3 Spec = BSDF(NV, NL, LH, NH, alpha, Specular.rgb);
	
	vec3 Diffuse = Albedo.rgb*vec3(Diff_Disney(NV, NL, LH, Roughness))/pi;

	vec3 lightModifier = lightColor.xyz * Attenuation;
	vec3 BSDFValue = Diffuse + Spec;
	return vec3(NL* BSDFValue * lightModifier);
}

float getShadowValue(in vec3 pos, in float nl) {
	vec4 shadow_coord = light.shadow_mat * vec4(pos,1);
	vec3 shadow_coords_final = shadow_coord.xyz / shadow_coord.w;
	float bias = 0.01*tan(acos(nl));
	bias = clamp(bias, 0, 0.01);
	bias /= shadow_coord.w;
	
	float sh = texture(shadow_map, shadow_coords_final.xy).r;
	float vis = 0;
	if (shadow_coords_final.x > 0 &&
		shadow_coords_final.x < 1 &&
		shadow_coords_final.y > 0 &&
		shadow_coords_final.y < 1)
	{
		if (sh > shadow_coords_final.z - bias) {
			vis = 1;
		}
	}

	return vis;
}

void main() {
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 Position = WorldPosFromDepth(Dist, fragTexCoord);
	vec3 Normal = texture(gbuffer1, fragTexCoord).rgb;
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;
	vec3 Specular = texture(gbuffer2, fragTexCoord).rgb;
	float Roughness = texture(gbuffer2, fragTexCoord).a;

	float lightAttenuationRadius = light.attenuationRadius;
    vec3 lightColor = light.color;
    float lightIntensity = light.power;
    vec3 lightPosition = light.position;

	vec3 lightPow = lightColor * lightIntensity;

	
	vec3 lightDir = normalize(Position-lightPosition);
	vec3 lightDirection = light.direction;

	float maxDot = cos(light.innerAngle);
	float minDot = cos(light.outerAngle);
	float dotPR = dot(lightDir, lightDirection);
	dotPR = clamp((dotPR-minDot)/(maxDot-minDot), 0, 1);

	float NL = clamp(dot(Normal, lightDirection), 0, 1);
	float sh = getShadowValue(Position, NL);

	vec3 outColor3 = vec3(0,0,0);
	if (dotPR > 0) {
		outColor3 = LightPointCalc(Albedo.rgb, Position.xyz, vec4(Specular, Roughness), Normal.xyz, lightPosition, lightAttenuationRadius, lightPow, ubo.eyePos.xyz) * dotPR;
		//outColor3 = hdrGammaTransform(outColor3);
	}

	outColor = vec4(sh * outColor3, 1);
}