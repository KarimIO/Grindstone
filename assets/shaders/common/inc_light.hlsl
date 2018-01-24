float3 linearToSRGB(float3 inColor) {
    float3 outColor;
	outColor.r = inColor.r <= 0.0031308 ? 12.92 * inColor.r : 1.055 * pow(inColor.r, 1.0/2.4) - 0.055;
	outColor.g = inColor.g <= 0.0031308 ? 12.92 * inColor.g : 1.055 * pow(inColor.g, 1.0/2.4) - 0.055;
	outColor.b = inColor.b <= 0.0031308 ? 12.92 * inColor.b : 1.055 * pow(inColor.b, 1.0/2.4) - 0.055;
	return outColor;
}

float3 hdrTransform(float3 color) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

	return clamp((color*(a*color+b))/(color*(c*color+d)+e), 0, 1);
}

float3 hdrTransformB(float3 color) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

	return clamp((color*(a*color+b))/(color*(c*color+d)+e), 0, 1);
}

float3 hdrGammaTransform(float3 color) {
	return hdrTransform(color); // linearToSRGB()
}

static const float pi = 3.14159f;

// Schlick
float3 Light_F(in float3 f0, in float f90, in float VH) {
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
	float3 f0 = float3(1.0f , 1.0f , 1.0f);
	float lightScatter = Light_F( f0 , fd90 , NdotL ) .r;
	float viewScatter = Light_F(f0 , fd90 , NdotV ).r;

	return lightScatter * viewScatter * energyFactor ;
}

float3 BSDF(float NV, float NL, float LH, float NH, float alpha, float3 SpecularColor) {
	float3 f0 = 0.32*SpecularColor*SpecularColor;
	float f90 = clamp(50 * dot(f0, float3(0.33, 0.33, 0.33)), 0, 1);
	
	float3 F = Light_F(f0, f90, LH);
	float D = Light_D(alpha, NH);
	float Vis = Light_V(NL, NV, alpha);

	return (F * D * Vis);
}

float3 LightPointCalc(in float3 Albedo, in float3 WorldPos, in float4 Specular, in float3 Normal, in float3 lightPos,
                        in float lightRadius, in float3 lightColor, in float3 eyePos) {

	float3 lightDir	= WorldPos - lightPos;
	float Distance	= length(lightDir);

	float3 eyeDir		= normalize(eyePos - WorldPos);
	float3 eyeReflect = reflect(-eyeDir, Normal);
	
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

	float3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(Normal, eyeDir)) + 0.00001;
	float NH = clamp(dot(Normal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);
	
	//float3 reflColor =  texture(texRefl, eyeReflect).rgb;

	float3 Spec = BSDF(NV, NL, LH, NH, alpha, Specular.rgb);
	
	float3 Diffuse = Albedo.rgb*Diff_Disney(NV, NL, LH, Roughness)/pi;

	float3 lightModifier = lightColor.xyz * Attenuation;
	float3 BSDFValue = Diffuse + Spec;
	return NL* BSDFValue * lightModifier;
}

float3 LightDirCalc(in float3 Albedo, in float3 WorldPos, in float3 lightDir, in float4 Specular, in float3 Normal, 
		in float3 lightColor, in float3 eyePos) {


	float3 eyeDir		= normalize(eyePos - WorldPos);
	float3 eyeReflect = reflect(-eyeDir, Normal);
	
	float Roughness = Specular.a;
	float alpha = Roughness * Roughness;
	float alphaSqr = alpha*alpha;

	float NL = clamp(dot(Normal, lightDir), 0, 1);
	
	float3 H = normalize(eyeDir + lightDir);
	
	float NV = abs(dot(Normal, eyeDir)) + 0.00001;
	float NH = clamp(dot(Normal, H), 0, 1);
	float LH = clamp(dot(lightDir, H), 0, 1);
	float VH = clamp(dot(eyeDir, H), 0, 1);
	
	float3 Spec = BSDF(NV, NL, LH, NH, alpha, Specular.rgb);
	
	float3 Diffuse = Albedo.rgb*Diff_Disney(NV, NL, LH, Roughness)/pi;

	float3 lightModifier = lightColor.xyz;
	float3 BSDFValue = Diffuse + Spec;
	return NL* BSDFValue * lightModifier;
}
