uniform mat4 invProjMat;
uniform mat4 invViewMat;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProjMat * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = invViewMat * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

vec2 OctWrap( vec2 v )
{
    return ( 1.0 - abs( v.yx ) ) * ( vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0) );
}
 
vec3 decodeNormal( vec3 enc )
{
    vec2 encN = enc.xy;
    encN = encN * 2.0 - 1.0;
 
    vec3 n;
    n.z = 1.0 - abs( encN.x ) - abs( encN.y );
    n.xy = (n.z >= 0.0) ? encN.xy : OctWrap( encN.xy );
    n = normalize( n );
    return n;
}