////////////////////////////////////////////////////////////////////////////////
// Filename: color.ps
////////////////////////////////////////////////////////////////////////////////

#pragma pack_matrix( column_major )

#include "../common/inc_transform.hlsl"
#include "../common/inc_light.hlsl"

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 viewRay : POSITION;
};

Texture2D shaderTexture[5];
SamplerState SampleType[5];

cbuffer MatrixInfoType {
    matrix invView;
    matrix invProj;
    float4 eyePos;
    float4 resolution;
};

cbuffer Light {
    matrix shadow_matrix;
	float3 lightPosition;
	float lightAttenuationRadius;
	float3 lightColor;
	float lightIntensity;
	float3 lightDirection;
	float lightInnerAngle;
	float lightOuterAngle;
};

float getShadowValue(in float3 pos, in float nl) {
	float4 shadow_coord = mul(shadow_matrix, float4(pos,1));
	float3 shadow_coords_final = shadow_coord.xyz / shadow_coord.w;
	float bias = 0.005*tan(acos(nl));
	bias = clamp(bias, 0, 0.01);
	bias /= shadow_coord.w;
	
	float sh = shaderTexture[4].Sample(SampleType[0], shadow_coords_final.xy).r;
	float vis = 0;
	if (shadow_coords_final.x > 0 &&
		shadow_coords_final.x < 1 &&
		shadow_coords_final.y > 0 &&
		shadow_coords_final.y < 1)
	{
	vis = sh;
	}


	return vis;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET {
    float  depth        = shaderTexture[3].Sample(SampleType[0], input.texCoord).r;
    float3 Position = WorldPosFromDepth(invProj, invView, depth, input.texCoord);
    
    float3 Albedo       = shaderTexture[0].Sample(SampleType[0], input.texCoord).rgb;
    float3 Normal       = shaderTexture[1].Sample(SampleType[0], input.texCoord).rgb;
    float4 Specular     = shaderTexture[2].Sample(SampleType[0], input.texCoord);
	
	float3 lightPow = lightColor * lightIntensity;
	float3 lightDir = normalize(Position-lightPosition);
	
	float nl = saturate(dot(Normal, lightDir));
    float sh = getShadowValue(Position, nl);
	
	float maxDot = cos(lightInnerAngle);
	float minDot = cos(lightOuterAngle);
	float dotPR = dot(lightDir, lightDirection);
	dotPR = clamp((dotPR-minDot)/(maxDot-minDot), 0, 1);
	float3 outColor = float3(0,0,0);
	if (dotPR > 0) {
		outColor = LightPointCalc(Albedo.rgb, Position.xyz, Specular, Normal.xyz, lightPosition, lightAttenuationRadius, lightPow, eyePos.xyz) * dotPR;
		outColor = hdrGammaTransform(outColor);
	}
	
	return float4(outColor, 1);
}