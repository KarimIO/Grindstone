#version 330 core

const float pi = 3.14159f;

out vec3 outColor;

in vec2 fragTexCoord;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform samplerCube environmentMap;

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec3 eyePos;
    vec2 resolution;
} ubo;

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

 float bitfieldReverse(uint bits) {
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
 }

vec2 Hammersley(uint i, uint N) {
	return vec2( float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10 );
}

float Light_D(in float alphaSqr, in float NH) {
	float denom = NH * NH * (alphaSqr - 1) + 1;

	return alphaSqr / (pi * denom * denom);
}

vec3 Light_F(in vec3 f0, in float f90, in float VH) {
	return f0 + (f90-f0) * pow(1-VH, 5.0f);
}

float Light_V( in float NL, in float NV, in float alphaSqr ) {
	float Lambda_GGXV = NL * sqrt (( - NV * alphaSqr + NV ) * NV + alphaSqr );
	float Lambda_GGXL = NV * sqrt (( - NL * alphaSqr + NL ) * NL + alphaSqr );

	return 0.25f / ( Lambda_GGXV + Lambda_GGXL );
}

vec3 MakeSample(vec2 E) {
	float SineTheta = sin(E.x);

	float x = cos(E.y) * SineTheta;
	float y = sin(E.y) * SineTheta;
	float z = cos(E.x);

	return vec3(x, y, z);
}

float compute_lod(float alphaSqr, uint NumSamples, float NH) {
	float dist = Light_D(alphaSqr, NH); // Defined elsewhere as subroutine
	return 0.5 * (log2(float(ubo.resolution.x * ubo.resolution.y) / NumSamples) - log2(dist));
}

vec3 radiance(vec3 N, vec3 V, vec4 Specular) {
	float alphaSqr = Specular.a * Specular.a;
	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 TangentX = normalize(cross( UpVector, N ));
	vec3 TangentY = cross(N, TangentX);

	float NoV = abs(dot(N, V));

	vec3 fColor = vec3(0.0);
	const uint NumSamples = 20u;
	for (uint i = 0u; i < NumSamples; ++i) {
		vec2 Xi = Hammersley(i, NumSamples);
		vec3 Li = MakeSample(Xi); // Defined elsewhere as subroutine
		vec3 H  = normalize(Li.x * TangentX + Li.y * TangentY + Li.z * N);
		vec3 L  = normalize(-reflect(V, H));

		// Calculate dot products for BRDF
		float NoL = abs(dot(N, L));
		float NoH = abs(dot(N, H));
		float VoH = abs(dot(V, H));
		float lod = compute_lod(alphaSqr, NumSamples, NoH);

		vec3 f0 = Specular.rgb;
		float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);

		vec3 F_ = Light_F(f0, f90, VoH); // Defined elsewhere as subroutine
		float G_ = Light_V( NoL, NoV, alphaSqr ); // Defined elsewhere as subroutine
		vec3 LColor = textureLod(environmentMap, L, lod).rgb;

		// Since the sample is skewed towards the Distribution, we don't need
		// to evaluate all of the factors for the lighting equation. Also note
		// that this function is calculating the specular portion, so we absolutely
		// do not add any more diffuse here.
		fColor += F_ * G_ * LColor;
	}

	// Average the results
	return fColor / float(NumSamples);
}

void main() {
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 Position = WorldPosFromDepth(Dist, fragTexCoord);
	vec3 Normal = texture(gbuffer1, fragTexCoord).xyz;
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;
	vec4 Specular = texture(gbuffer2, fragTexCoord);


    vec3 v = normalize(ubo.eyePos.xyz - Position);
    vec3 r = reflect(v, Normal);
   
    vec3 refl = texture(environmentMap, r).rgb;

    vec3 ambientColor = vec3(0.9f, 0.96f, 1.0f) * 0.05f;
	outColor = Specular.rgb * refl;

	/*vec3 V = normalize(ubo.eyePos - Position);

	vec3 N = Normal;
	vec3 L = normalize(-reflect(V, N));
	float NoV = clamp(dot(N, V), 0, 1);
	float NoL = clamp(dot(N, L), 0, 1);

	// Calculate different portions of the color
	vec3 eyeDir = normalize(ubo.eyePos - Position);
	vec3 eyeRefl= reflect(-eyeDir, Normal);

	vec3 irrMap = texture(environmentMap, eyeRefl).rgb;
	vec3 Kdiff  = irrMap * Albedo / pi;
	vec3 Kspec  = radiance(N, V, Specular);

	// Mix the materials
	outColor = Kspec + Kdiff;*/
}