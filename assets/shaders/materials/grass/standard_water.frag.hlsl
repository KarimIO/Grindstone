////////////////////////////////////////////////////////////////////////////////
// Filename: color.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType {
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD0;
};

Texture2D shaderTexture[4];
SamplerState SampleType[4];

float3 CalcBumpedNormal(float3 Ng, float3 Nt, float3 Tan) {
  float3 BumpMapNormal = Nt;
  //if (BumpMapNormal == float3(0, 0, 0))
  //  return Ng;
  float3 NewTangent = normalize(Tan - dot(Tan, Ng) * Ng);
  float3 Bitangent = cross(NewTangent, Ng);
  BumpMapNormal = 2.0 * BumpMapNormal - float3(1.0, 1.0, 1.0);
  BumpMapNormal = float3(-BumpMapNormal.xy, BumpMapNormal.z);
  float3x3 TBN = float3x3(NewTangent, Bitangent, Ng);
  BumpMapNormal = mul(TBN, BumpMapNormal);
  return normalize(BumpMapNormal);
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_Target0 {
    return float4(1.0,0.8,0.0,0.4);
}