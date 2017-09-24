float4 ViewPosFromDepth(matrix invProj, float depth, float2 TexCoord) {
    float z = depth;

    TexCoord = TexCoord * 2.0 - 1.0;
    TexCoord.y *= -1;
    float4 clipSpacePosition = float4(TexCoord, z, 1.0);
    float4 viewSpacePosition = mul(invProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

float3 WorldPosFromViewPos(matrix invView, float4 view) {
    float4 worldSpacePosition = mul(invView, view);

    return worldSpacePosition.xyz;
}

float3 WorldPosFromDepth(matrix invProj, matrix invView, float depth, float2 TexCoord) {
    return WorldPosFromViewPos(invView, ViewPosFromDepth(invProj, depth, TexCoord));
}