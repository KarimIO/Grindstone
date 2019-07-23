#version 330 core

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
in vec3 fragPosition;
in vec3 fragNormal;
out vec4 outColor;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eyepos;
    float time;
    mat4 invProj;
    mat4 invView;
	vec4 resolution;
} ubo;

const float pi = 3.14159f;

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

float Light_F(in float nl) {
    float fresnel = pow(1-nl, 5.0f);
	// float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);

    return fresnel;
}

vec3 BSDF(float NV, float NL, float LH, float NH, float alpha, vec3 SpecularColor) {
	vec3 f0 = 0.32*SpecularColor*SpecularColor;
	float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);
	
	vec3 F = /*f0 + (f90-f0) * */ f0 * Light_F(LH);
	float D = Light_D(alpha, NH);
	float Vis = Light_V(NL, NV, alpha);

	return (F * D * Vis);
}

vec3 getLight(in vec3 lightDir, in vec3 lightColor) {
    vec3 eyeDir		= normalize(ubo.eyepos - fragPosition);
	vec3 eyeReflect = reflect(-eyeDir, fragNormal);
	
    vec4 Specular = vec4(0.3, 0.6, 0.9, 0.05f);

	float Roughness = Specular.a;
	float alpha = Roughness * Roughness;
	float alphaSqr = alpha*alpha;

	float NL = clamp(dot(fragNormal, lightDir), 0, 1);

	vec3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(fragNormal, eyeDir)) + 0.00001;
	float NH = clamp(dot(fragNormal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);

	vec3 Spec = BSDF(NV, NL, LH, NH, alpha, Specular.rgb);
	
	return vec3(NL * Spec * lightColor);
}

vec3 getWaterFresnel() {
    vec3 eyedir = normalize(ubo.eyepos - fragPosition);
    float nl = clamp(dot(eyedir, fragNormal), 0, 1);

    vec3 color = vec3(0.3, 0.6, 0.9);
    // vec3 f0 = 0.32*color*color;
	
	return color * Light_F(nl);
}

vec3 getRefraction(in vec2 uv) {
	float dist = length(ubo.eyepos - fragPosition);
	dist = log(dist / 4);
    dist = clamp(dist, 0, 1);
	
    vec3 eyeDir		= normalize(ubo.eyepos - fragPosition);
	vec3 eyeReflect = reflect(-eyeDir, fragNormal);

	vec3 depthColor = vec3(0.0, 0.1, 0.2);
	vec2 refUV = uv; // + fragNormal.xz * 0.1;
	
	vec3 refrColor = texture(gbuffer0, refUV).xyz;
	return refrColor; //mix(refrColor, depthColor, 0); //dist);
}

void main() {
	vec2 tc = textureSize(gbuffer0, 0);
	vec2 uv = gl_FragCoord.xy / tc;
	float depth = texture(gbuffer3, uv).r;
	vec3 p = WorldPosFromDepth(depth, uv);
	depth = length(p - fragPosition);
	
	float dist = length(ubo.eyepos - fragPosition);
	float diff = dist - depth;

    vec3 lightDir = -normalize(vec3(-1, -1, -1));
    vec3 lightColor = vec3(1.0f, 0.95f, 0.9f) * 5.0f; // * 10.0f;
    vec3 color = getRefraction(uv) + getWaterFresnel() + getLight(lightDir, lightColor);
    outColor = vec4(color, 1); //  + color
}