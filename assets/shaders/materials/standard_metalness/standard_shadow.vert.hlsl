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

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(VertexInputType input) : SV_POSITION {
    float4 position = float4(input.position, 1.0f);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    position = mul(position, worldMatrix);
    position = mul(position, proj_view);
    
    return position;
}