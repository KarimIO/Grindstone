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
	float3 lightPosition;
    float lightAttenuationRadius;
	float3 lightColor;
	float lightIntensity;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET {
    float  depth        = shaderTexture[3].Sample(SampleType[0], input.texCoord).r;
    float3 Position = WorldPosFromDepth(invProj, invView, depth, input.texCoord);
    //return float4(position, 1.0);
    /*float near = 0.1;
    float far = 100;
    float ProjectionA = far / (far - near);
    float ProjectionB = (-far * near) / (far - near);
    depth = ProjectionB / ((depth - ProjectionA));
    float4 position = float4(input.viewRay * depth, 1.0);*/
    // Convert to World Space:
    // position = mul(invView, position);
    float3 Albedo       = shaderTexture[0].Sample(SampleType[0], input.texCoord).rgb;
    float3 Normal       = shaderTexture[1].Sample(SampleType[0], input.texCoord).rgb;
    float4 Specular     = shaderTexture[2].Sample(SampleType[0], input.texCoord);

	float3 lightPow = lightColor * lightIntensity;
	float3 outColor = LightPointCalc(Albedo.rgb, Position.xyz, Specular, Normal.xyz, lightPosition, lightAttenuationRadius, lightPow, eyePos.xyz); // hdrGammaTransform()
	return float4(hdrGammaTransform(outColor), 1.0f);
}