#pragma pack_matrix( column_major )

Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

cbuffer MatrixInfoType {
    float aspect;
    float time;
}

float Dist(float2 d) {
    return sqrt(d.x*d.x + d.y*d.y);
}

float CalcSphere(float2 pos, float size) {
    float2 pos2 = float2(pos.x * aspect, pos.y);
    float dist = Dist(pos2);
    return (dist < size) ? 1 : 0;
}

float4 main(PixelInputType input) : SV_TARGET {
    float2 res = float2(1366, 768);
    float h = res.y * 0.28;
    float2 picCoord = float2((res.x - h), (res.y - h))/2;
    float2 texCoord = input.texCoord * res;
    texCoord -= picCoord;
    texCoord /= h;

    float4 color = shaderTexture.Sample(SampleType, texCoord);
    if (texCoord.x < 0 || texCoord.x > 1 || texCoord.y < 0 || texCoord.y > 1)
        color = float4(0,0,0,1);

    float4 background = float4(0.211, 0.306, 0.341, 1.0);
    float cs = 0.0f;
    if (time > 0.0f) {
        const float circle_dist = 0.15;
        const uint num_orbs = 10;
        float offset = sin(time * 2 / 3.14159) * 0.5 + 1;
        for (uint i = 0; i < num_orbs; i++) {
            float f = float(i) / num_orbs;
            float off = time + i;
            off *= -0.2;
            float2 circle_pos = float2(0.5 + cos(off) * circle_dist / aspect, 0.5 + sin(off) * circle_dist);
            cs += CalcSphere(circle_pos - input.texCoord, 0.005 * sqrt(f)) * f;
        }
    }
    float4 c = float4(cs, cs, cs, 1);
    return c + background + color;
}