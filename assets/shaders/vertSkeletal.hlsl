////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
#pragma pack_matrix( row_major )

cbuffer MatrixBuffer {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

const uint MAX_BONES = 100;

cbuffer BoneBuffer {
    matrix bones[MAX_BONES];
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
    uint4 boneIDs : BLENDINDICES0;
    float4 boneWeights : BLENDWEIGHT0;
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
    
    matrix animMat = bones[input.boneIDs[0]] * input.boneWeights[0];
    animMat += bones[input.boneIDs[1]] * input.boneWeights[1];
    animMat += bones[input.boneIDs[2]] * input.boneWeights[2];
    animMat += bones[input.boneIDs[3]] * input.boneWeights[3];

    // Change the position vector to be 4 units for proper matrix calculations.
    position = float4(input.position, 1.0f);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    position = mul(position, animMat);
    position = mul(position, worldMatrix);
    output.worldPosition = position.xyz;
    position = mul(position, viewMatrix);
    output.position = mul(position, projectionMatrix);
    
    float4 normalIn = float4(input.normal, 1.0);
    normalIn = mul(normalIn, animMat);
    normalIn = mul(normalIn, worldMatrix);
    output.normal = normalize(normalIn.xyz);
    output.tangent = normalize(mul(float4(input.tangent, 1.0), worldMatrix).xyz);
    output.texCoord = float2(input.texCoord.x, -input.texCoord.y);
    
    return output;
}