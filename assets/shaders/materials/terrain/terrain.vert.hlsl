////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#pragma pack_matrix( row_major )

cbuffer MatrixBuffer
{
    matrix proj_view;
};

cbuffer ModelMatrixBuffer {
    matrix worldMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
    float2 position : POSITION;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
};

Texture2D shaderTexture;
SamplerState sampleType;

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input) {
    float4 position;
    PixelInputType output;

    float2 tex = (input.position)/240.0f;
    float heightmap = shaderTexture.SampleLevel(sampleType, tex, 0).r * 20.0f;

    // Change the position vector to be 4 units for proper matrix calculations.
    position = float4(input.position.x, heightmap, input.position.y, 1.0f);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    position = mul(position, worldMatrix);
    output.worldPosition = position.xyz;
    output.position = mul(position, proj_view);
    
    float3 n = float3(0.0, 1.0, 0.0);
    float3 t = float3(0.0, 1.0, 0.0);
    output.normal = normalize(mul(float4(n, 1.0), worldMatrix).xyz);
    output.tangent = normalize(mul(float4(t, 1.0), worldMatrix).xyz);
    output.texCoord = tex;
    
    return output;
}