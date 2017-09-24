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

struct PixelOutputType {
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 specular : SV_Target2;
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
PixelOutputType main(PixelInputType input) {
    PixelOutputType output;
    
    float metalness = shaderTexture[3].Sample(SampleType[3], input.texCoord).r;

    output.albedo = (1 - metalness) * shaderTexture[0].Sample(SampleType[0], input.texCoord);
    float3 normalTex;
    normalTex.rg = shaderTexture[1].Sample(SampleType[1], input.texCoord).rg;
    normalTex.b = sqrt(normalTex.r*normalTex.r + normalTex.g*normalTex.g);

    output.normal.rgb = CalcBumpedNormal(input.normal, normalTex, input.tangent);
    output.normal.a = 1;

    output.specular.a = shaderTexture[2].Sample(SampleType[2], input.texCoord).r;
    output.specular.rgb = lerp(float3(0.04, 0.04, 0.04), output.albedo.rgb, metalness);
    return output;
}