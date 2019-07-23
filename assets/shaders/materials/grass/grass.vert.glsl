#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eyepos;
    float time;
    mat4 invProj;
    mat4 invView;
	vec4 resolution;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec2 vertexTexCoord;

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

void main() {
    // mbo.model messes up everything for some reason
    vec3 windDir = normalize(vec3(-1, 0, -1));
    float fWindStrength = 10.0f;
    float fWindPower = 0.5f+sin(vertexPosition.x/30+vertexPosition.z/30+ubo.time*(1.2f+fWindStrength/20.0f)+cos(ubo.time*(0.6f+fWindStrength/10.0f)));

    if(fWindPower < 0.0f)
        fWindPower = fWindPower*0.2f;
    else fWindPower = fWindPower*0.3f;
    vec3 offsetPos = fWindPower * windDir * clamp(vertexTexCoord.y * vertexTexCoord.y, 0, 1) ;
    
    fragPosition = (mbo.model * vec4(offsetPos + vertexPosition, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragNormal = normalize((mbo.model * vec4(vertexNormal, 0.0)).xyz);
    fragTangent =  normalize((mbo.model * vec4(vertexTangent, 0.0)).xyz);
    fragTexCoord = vec2(vertexTexCoord.x, -vertexTexCoord.y);
}