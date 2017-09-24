////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#pragma pack_matrix( row_major )

cbuffer MatrixBuffer
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ModelMatrixBuffer {
    matrix worldMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input) {
    float4 position;
    PixelInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    position = float4(input.position, 1.0f);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    position = mul(position, worldMatrix);
    output.worldPosition = position.xyz;
    position = mul(position, viewMatrix);
    output.position = mul(position, projectionMatrix);
    
    output.normal = normalize(mul(float4(input.normal, 1.0), worldMatrix).xyz);
    output.tangent = normalize(mul(float4(input.tangent, 1.0), worldMatrix).xyz);
    output.texCoord = float2(input.texCoord.x, -input.texCoord.y);
    
    return output;
}