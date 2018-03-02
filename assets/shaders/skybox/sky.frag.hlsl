////////////////////////////////////////////////////////////////////////////////
// Filename: color.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType {
    float4 position : SV_POSITION;
    float3 coord : POSITION;
};

TextureCube skybox;
SamplerState SampleType;

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_Target0 {
    float l = length(input.coord);
    float3 coord = input.coord/l;
    float3 color = skybox.Sample(SampleType, coord).rgb;

    return float4(color, 1.0);
}