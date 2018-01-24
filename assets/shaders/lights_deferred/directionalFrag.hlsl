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

Texture2D shaderTexture[4];
SamplerState SampleType[4];

cbuffer MatrixInfoType {
    matrix invView;
    matrix invProj;
    float4 eyePos;
    float4 resolution;
};

cbuffer Light {
	float3 lightDirection;
    float lightSunRadius;
	float3 lightColor;
	float lightIntensity;
};

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
	float3 outColor = LightDirCalc(Albedo.rgb, Position.xyz, lightDirection, Specular, Normal.xyz, lightPow, eyePos.xyz);
	return float4(hdrGammaTransform(outColor), 1.0f);
}