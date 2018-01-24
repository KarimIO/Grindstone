#version 410 core

layout(triangles, equal_spacing, cw) in;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

uniform sampler2D heightmap;

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
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
    vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
    vec4 position = mix(a, b, v);
    
    fragTexCoord = position.xy;
    float height = 0.0;
    gl_Position = ubo.proj_view * mbo.model * vec4(fragTexCoord.x, height, fragTexCoord.y, 1);

    fragNormal = vec3(0,1,0);
    fragTangent = fragNormal;
}