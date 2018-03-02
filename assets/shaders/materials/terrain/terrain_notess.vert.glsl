#version 330 core

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

in vec2 vertexPosition;

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

uniform sampler2D heightmap;

// # P.xy store the position for which we want to calculate the normals
// # height() here is a function that return the height at a point in the terrain

// read neightbor heights using an arbitrary small offset
vec3 getNormals(vec2 P) {
    vec3 off = vec3(1.0, 1.0, 0.0) / 512;
    float hL = texture(heightmap, P.xy - off.xz).r * 20.0f;
    float hR = texture(heightmap, P.xy + off.xz).r * 20.0f;
    float hD = texture(heightmap, P.xy - off.zy).r * 20.0f;
    float hU = texture(heightmap, P.xy + off.zy).r * 20.0f;

    // deduce terrain normal
    vec3 N;
    N.x = hL - hR;
    N.y = hD - hU;
    N.z = 2.0;
    N = normalize(N);
    return N;
}

void main() {
    vec2 tex = (vertexPosition)/240.0f;
    float heightmap = texture(heightmap, tex).r * 20.0f;

    // mbo.model messes up everything for some reason
    fragPosition = (mbo.model * vec4(vertexPosition.x, heightmap, vertexPosition.y, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragNormal = getNormals(tex).xyz;
    fragTangent =  vec3(0,1,0);
    fragTexCoord = tex;
}