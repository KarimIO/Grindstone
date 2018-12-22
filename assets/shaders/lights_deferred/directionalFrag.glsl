#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

layout(std140) uniform Light {
	mat4 shadow_mat;
	vec3 direction;
	float sourceRadius;
	vec3 color;
	float power;
	bool shadow;
} light;

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform sampler2DShadow shadow_map;

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

vec3 LightDirCalc(in vec3 Albedo, in vec3 WorldPos, in vec3 lightDir, in vec4 Specular, in vec3 Normal, in vec3 lightColor, in vec3 eyePos) {
	vec3 eyeDir		= normalize(eyePos - WorldPos);
	vec3 eyeReflect = reflect(-eyeDir, Normal);
	
	float Roughness = Specular.a;
	float alpha = Roughness * Roughness;
	float alphaSqr = alpha*alpha;

	float NL = clamp(dot(Normal, lightDir), 0, 1);
	
	vec3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(Normal, eyeDir)) + 0.00001;
	float NH = clamp(dot(Normal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);

	vec3 Spec = BSDF(NV, NL, LH, NH, alpha, Specular.rgb);
	
	vec3 Diffuse = Albedo.rgb*vec3(Diff_Disney(NV, NL, LH, Roughness))/pi;

	vec3 lightModifier = lightColor.xyz;
	vec3 BSDFValue = Diffuse + Spec;
	return vec3(NL* BSDFValue * lightModifier);
}

//in vec3 viewRay;

float shadowRandom(vec4 seed4) {
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
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

float getShadowValue(in vec3 pos, in float nl) {
	vec4 shadow_coord = light.shadow_mat * vec4(pos,1);
	float bias = 0.005; //*tan(acos(nl));
	bias = clamp(bias, 0, 0.01);
	
	float visibility = 1.0f;
	for (int i=0;i<4;i++){
        int index = int(16.0*shadowRandom(vec4(floor(pos.xyz*1000.0), i)))%16;
        visibility -= 0.25*(1.0-texture( shadow_map, vec3(shadow_coord.xy + poissonDisk[index]/700.0, shadow_coord.z - bias)));
    }

	return visibility;
}

void main() {
	float depth = texture(gbuffer3, fragTexCoord).r;
	float near = 0.1;
    float far = 100;
    float ProjectionA = far / (far - near);
    float ProjectionB = (-far * near) / (far - near);
    //depth = ProjectionB / ((depth - ProjectionA));
    //vec3 position = viewRay * depth;

	vec3 Position = WorldPosFromDepth(depth, fragTexCoord);
	vec3 Normal = texture(gbuffer1, fragTexCoord).rgb;
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;
	vec3 Specular = texture(gbuffer2, fragTexCoord).rgb;
    float Roughness = texture(gbuffer2, fragTexCoord).a;

	float lightSourceRadius = light.sourceRadius;
    vec3 lightColor = light.color;
    float lightIntensity = light.power;
    vec3 lightDir = light.direction;
	float nl = dot(lightDir, Normal);

	vec3 lightPow = lightColor * lightIntensity;
	vec3 outColor3 = LightDirCalc(Albedo.rgb, Position, lightDir.xyz, vec4(Specular, Roughness), Normal.xyz, lightPow, ubo.eyePos.xyz);
	
	float sh = light.shadow ? getShadowValue(Position, nl) : 1.0f;
	outColor = vec4(sh * outColor3, 1);
}