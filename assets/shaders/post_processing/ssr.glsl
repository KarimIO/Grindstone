#version 400 core

/*uniform sampler2D gFinalImage;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gExtraComponents;
uniform sampler2D ColorBuffer;*/

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform sampler2D lighting;

layout(std140) uniform DefferedUBO {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition.xyz /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec4 ViewPosFromUV(vec2 TexCoord) {
	float Dist = texture(gbuffer3, TexCoord).r;
	return ViewPosFromDepth(Dist, TexCoord);
}


vec3 ViewNormal(vec3 inNorm) {
    return (transpose(ubo.invView) * normalize(vec4(inNorm, 1.0))).rgb;
}

in vec2 fragTexCoord; // noperspective

out vec4 outColor;

const float step = 0.1;
const float minRayStep = 0.1;
const float maxSteps = 30;
const int numBinarySearchSteps = 5;
const float reflectionSpecularFalloffExponent = 3.0;

float Metallic;

#define Scale vec3(.8, .8, .8)
#define K 19.19

vec3 PositionFromDepth(float depth);

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);
 
vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth);

vec3 Light_F(in vec3 f0, in float f90, in float VH) {
	return f0 + (f90-f0) * pow(1-VH, 5.0f);
}

vec3 hash(vec3 a);

void main()
{
	vec4 Position = ViewPosFromUV(fragTexCoord);
	vec3 Normal = ViewNormal(texture(gbuffer1, fragTexCoord).xyz);
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;
	vec4 Specular = texture(gbuffer2, fragTexCoord);

    /*vec2 MetallicEmmissive = texture2D(gExtraComponents, fragTexCoord).rg;
    Metallic = MetallicEmmissive.r;

    if(Metallic < 0.01)
        discard;
 
    vec3 viewNormal = vec3(texture2D(gNormal, fragTexCoord) * ubo.invView);
    vec3 viewPos = textureLod(gPosition, fragTexCoord, 2).xyz;
    vec3 albedo = texture(gFinalImage, fragTexCoord).rgb;

    float spec = texture(ColorBuffer, fragTexCoord).w;

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, Metallic);
    vec3 Fresnel = fresnelSchlick(max(dot(normalize(viewNormal), normalize(viewPos)), 0.0), F0);*/
    
    vec3 f0 = Specular.rgb;
    float Metallic = dot(f0, vec3(0.33));

    if (Metallic < 0.01) {
        discard;
    }

    float f90 = clamp(50 * Metallic, 0, 1);

    float VoH = 1 - max(dot(normalize(Normal), normalize(Position.xyz)), 0.0f);
    vec3 Fresnel = clamp(Light_F(f0, f90, VoH), 0, 1);

    // Reflection vector
    vec3 reflected = normalize(reflect(normalize(Position.xyz), normalize(Normal)));
    float spec = Specular.w;

    vec3 hitPos = Position.xyz;
    float dDepth;
 
    bool found = true;
    vec3 wp = WorldPosFromViewPos(Position);
    vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), spec);
    vec4 coords = RayMarch((jitt + reflected * max(minRayStep, -Position.z)), hitPos, dDepth); // vec3(jitt) + 
    
    if (coords.x < 0.0f || coords.x > 1.0f || coords.y < 0.0f || coords.y > 1.0f) {
        found = false;
    }
 
    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
 
 
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    float ReflectionMultiplier = pow(Metallic, reflectionSpecularFalloffExponent) * 
                screenEdgefactor * 
                -reflected.z;
 
    // Get color
    vec3 SSR = textureLod(lighting, coords.xy, 0).rgb * clamp(screenEdgefactor, 0.0, 0.9); // * Fresnel;  

    float factor = found ? 1.0f : 0.0f;
    outColor = vec4(clamp(SSR * factor, 0, 1), factor);
}

vec3 PositionFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(fragTexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    float depth;

    vec4 projectedCoord;
 
    for(int i = 0; i < numBinarySearchSteps; i++)
    {

        projectedCoord = inverse(ubo.invProj) * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        //depth = textureLod(gPosition, projectedCoord.xy, 2).z;
	    depth = ViewPosFromUV(projectedCoord.xy).z;

 
        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            hitCoord += dir;
        else
            hitCoord -= dir;    
    }

        projectedCoord = inverse(ubo.invProj) * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
    return vec3(projectedCoord.xy, depth);
}

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{

    dir *= step;
 
 
    float depth;
    int steps;
    vec4 projectedCoord;

 
    for(int i = 0; i < maxSteps; i++)
    {
        hitCoord += dir;
 
        projectedCoord = inverse(ubo.invProj) * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        // depth = textureLod(gPosition, projectedCoord.xy, 2).z;
	    depth = ViewPosFromUV(projectedCoord.xy).z;

        if(depth > 1000.0)
            continue;
 
        dDepth = hitCoord.z - depth;

        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
            {   
                vec4 Result;
                Result = vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);

                return Result;
            }
        }
        
        steps++;
    }
 
    
    return vec4(projectedCoord.xy, depth, 0.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}