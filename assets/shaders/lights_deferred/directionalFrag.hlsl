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
	float3 lightDirection;
    float lightSunRadius;
	float3 lightColor;
	float lightIntensity;
};

float getShadowValue(in float3 pos, in float nl) {
	float4 shadow_coord = mul(shadow_matrix, float4(pos,1));
	float bias = 0.005*tan(acos(nl));
	bias = clamp(bias, 0, 0.01);
	bias /= shadow_coord.w;
	
	float sh = shaderTexture[4].Sample(SampleType[0], shadow_coord.xy).r;
	float vis = 0;
	if (shadow_coord.x > 0 &&
		shadow_coord.x < 1 &&
		shadow_coord.y > 0 &&
		shadow_coord.y < 1)
	{
		if (sh > shadow_coords_final.z - bias) {
			vis = 1;
		}
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

    float nl = saturate(dot(Normal, lightDirection));
    float sh = getShadowValue(Position, nl);
    float3 lightPow = lightColor * lightIntensity;
	float3 outColor = LightDirCalc(Albedo.rgb, Position.xyz, lightDirection, Specular, Normal.xyz, lightPow, eyePos.xyz);
	return float4(lightDirection * 0.5 + float3(0.5, 0.5, 0.5), 1.0f);
}