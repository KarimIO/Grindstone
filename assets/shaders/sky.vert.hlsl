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
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float3 coord : POSITION;
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
    //position = position;
    output.coord = position.xyz;
    output.position = mul(position, proj_view);
    output.position.z = output.position.w - 10;
    
    return output;
}