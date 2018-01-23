////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////

//////////////
// TYPEDEFS //
//////////////
#pragma pack_matrix( row_major )

struct VertexInputType {
    float2 position : POSITION;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 viewRay : POSITION;
};

cbuffer MatrixInfoType {
    float4x4 invView;
    float4x4 invProj;
    float4 eyePos;
    float4 resolution;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input) {
    PixelInputType output;

    output.texCoord = float2(input.position.x, -input.position.y);
	output.texCoord = (output.texCoord + float2(1,1)) / 2.0;
	output.position =  float4(input.position, 0.0, 1.0);

    float3 positionVS = mul(float4(input.position, 0.0, 1.0), invProj).xyz;
    output.viewRay = float3(positionVS.xy / positionVS.z, 1.0f);
    
    return output;
}