#version 400 core

in vec2 fragTexCoord;
out vec4 outval;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;
uniform sampler2D lighting;

layout(std140) uniform UniformBufferObject {
    mat4 invView;
    mat4 invProj;
    vec3 eyePos;
    vec2 resolution;
	float time;
} ubo;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

const float step = 0.1;
const float minRayStep = 0.1;
const float maxSteps = 30;
const int numBinarySearchSteps = 5;
const float reflectionSpecularFalloffExponent = 3.0;

float Metallic;

#define Scale vec3(.8, .8, .8)
#define K 19.19

mat4 getProjection() {
    return inverse(ubo.invProj);
}

vec3 PositionFromDepth(float depth);

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);
 
vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth);

vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 hash(vec3 a);


vec3 Light_F(in vec3 f0, in float f90, in float VH) {
	return f0 + (f90-f0) * pow(1-VH, 5.0f);
}

void main()
{
    float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 position = ViewPosFromDepth(Dist, fragTexCoord).xyz;
	vec3 normal = texture(gbuffer1, fragTexCoord).xyz;
	vec3 light = texture(lighting, fragTexCoord).xyz;
    vec4 specrough = texture(gbuffer2, fragTexCoord).xyzw;
    float specular = specrough.r;

    if(specular == 0.0) {
        discard;
    }

    outval = vec4(normal, 1);

    /*vec2 MetallicEmmissive = texture2D(gExtraComponents, TexCoords).rg;
    Metallic = MetallicEmmissive.r;

    if(Metallic < 0.01)
        discard;
 
    vec3 viewNormal = vec3(texture2D(gNormal, TexCoords) * invView);
    vec3 viewPos = textureLod(gPosition, TexCoords, 2).xyz;
    vec3 albedo = texture(gFinalImage, TexCoords).rgb;

    float spec = texture(ColorBuffer, TexCoords).w;

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, Metallic);
    vec3 Fresnel = fresnelSchlick(max(dot(normalize(viewNormal), normalize(viewPos)), 0.0), F0);
    */
    
    vec3 f0 = vec3(specular); //Specular.rgb;
    float f90 = clamp(50 * dot(f0, vec3(0.33)), 0, 1);

	float VoH = max(dot(normalize(normal), normalize(position)), 0.0);
    
    float Metallic = 1;
    vec3 Fresnel = fresnelSchlick(max(dot(normalize(normal), normalize(position)), 0.0), f0);

    // Reflection vector
    vec3 reflected = normalize(reflect(normalize(position), normalize(normal)));

    vec3 hitPos = position;
    float dDepth;
 
    vec3 wp = vec3(vec4(position, 1.0) * ubo.invView);
    vec3 jitt = vec3(0); //mix(vec3(0.0), vec3(hash(wp)), specular);
    vec4 coords = RayMarch((vec3(jitt) + reflected * max(minRayStep, -position.z)), hitPos, dDepth);
 
 
    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
 
 
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    float ReflectionMultiplier = pow(Metallic, reflectionSpecularFalloffExponent) * 
                screenEdgefactor * 
                -reflected.z;
 
    // Get color
    vec3 SSR = texture(gbuffer0, coords.xy).rgb * Fresnel; // * clamp(ReflectionMultiplier, 0.0, 0.9) * Fresnel;  

    outval = vec4(coords.xy, 0, 1); // Metallic);
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

        projectedCoord = getProjection() * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        depth = textureLod(gbuffer3, projectedCoord.xy, 2).r;

 
        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            hitCoord += dir;
        else
            hitCoord -= dir;    
    }

        projectedCoord = getProjection() * vec4(hitCoord, 1.0);
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
 
        projectedCoord = getProjection() * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        depth = textureLod(gbuffer3, projectedCoord.xy, 2).r;
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