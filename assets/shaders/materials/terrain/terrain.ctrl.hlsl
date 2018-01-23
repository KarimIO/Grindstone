struct HullOut
{
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex      : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut main(InputPatch<VertexOut,3> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HullOut hout;
    
    // Pass through shader.
    hout.PosW     = p[i].PosW;
    hout.NormalW  = p[i].NormalW;
    hout.TangentW = p[i].TangentW;
    hout.Tex      = p[i].Tex;
    
    return hout;
}