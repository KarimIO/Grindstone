#version 330 core

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
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 ViewNormal(vec3 inNorm) {
    return (transpose(ubo.invView) * normalize(vec4(inNorm, 1.0))).rgb;
}

bool traceScreenSpaceRay(
    // Camera-space ray origin, which must be within the view volume
    vec3 csOrig,
    // Unit length camera-space ray direction
    vec3 csDir,
    // Number between 0 and 1 for how far to bump the ray in stride units
    // to conceal banding artifacts. Not needed if stride == 1.
    float jitter,
    // Pixel coordinates of the first intersection with the scene
    out vec2 hitPixel,
    // Camera space location of the ray hit
    out vec3 hitPoint) {

        return false;

}

void main() {
	float Dist = texture(gbuffer3, fragTexCoord).r;
	vec3 position = ViewPosFromDepth(Dist, fragTexCoord).xyz;
	vec3 normal = texture(gbuffer1, fragTexCoord).xyz;
	vec3 light = texture(lighting, fragTexCoord).xyz;
    //normal = ViewNormal(normal);

    vec3 csOrig = vec3(position.xy, 0);
    vec3 csDir = position;
    float jitter = 1.0f;

    vec2 hitPixel;
    vec3 hitPoint;
    traceScreenSpaceRay(csOrig, csDir, jitter, hitPixel, hitPoint);

    outval = vec4(light, 1);
}