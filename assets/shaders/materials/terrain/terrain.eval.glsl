#version 410 core

layout(triangles, equal_spacing, ccw) in;

//uniform mat4 gVP;
//uniform sampler2D gDisplacementMap;
//uniform float gDispFactor;


layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eye_pos;
} ubo;

layout(std140) uniform ModelMatrixBuffer {
    mat4 model;
} mbo;

uniform sampler2D heightmap;

in vec3 WorldPos_ES_in[];
in vec2 TexCoord_ES_in[];
in vec3 Normal_ES_in[];

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec2 fragTexCoord;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2) {
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
} 

vec3 estimateNormal(vec2 texcoord) {
    float HEIGHT = 4096.0f;
    float WIDTH = 4096.0f;
    float SCALE = 1.0f; //displace_ratio;

    vec2 uv =  texcoord;
    vec2 du = vec2(1/WIDTH, 0);
    vec2 dv= vec2(0, 1/HEIGHT);
    float dhdu = SCALE/(2/WIDTH) * (texture(heightmap, uv+du).r - texture(heightmap, uv-du).r);
    float dhdv = SCALE/(2/HEIGHT) * (texture(heightmap, uv+dv).r - texture(heightmap, uv-dv).r);

    vec3 N = normalize(vec3(0,1,0) + vec3(0,0,1) * dhdu + vec3(1,0,0) * dhdv);

    return N;

    float width = 4096.0f;
    float depth = 4096.0f;
    float scale = 1.0f;

	vec2 b = texcoord + vec2(0.0f, -0.3f / depth);
	vec2 c = texcoord + vec2(0.3f / width, -0.3f / depth);
	vec2 d = texcoord + vec2(0.3f / width, 0.0f);
	vec2 e = texcoord + vec2(0.3f / width, 0.3f / depth);
	vec2 f = texcoord + vec2(0.0f, 0.3f / depth);
	vec2 g = texcoord + vec2(-0.3f / width, 0.3f / depth);
	vec2 h = texcoord + vec2(-0.3f / width, 0.0f);
	vec2 i = texcoord + vec2(-0.3f / width, -0.3f / depth);

	float zb = texture(heightmap, b).x * scale;
	float zc = texture(heightmap, c).x * scale;
	float zd = texture(heightmap, d).x * scale;
	float ze = texture(heightmap, e).x * scale;
	float zf = texture(heightmap, f).x * scale;
	float zg = texture(heightmap, g).x * scale;
	float zh = texture(heightmap, h).x * scale;
	float zi = texture(heightmap, i).x * scale;

	float x = zg + 2 * zh + zi - zc - 2 * zd - ze;
	float y = 2 * zb + zc + zi - ze - 2 * zf - zg;
	float z = 8.0f;

	return normalize(vec3(x, y, z));
}

void main() {
    // Interpolate the attributes of the output vertex using the barycentric coordinates
    fragTexCoord = interpolate2D(TexCoord_ES_in[0], TexCoord_ES_in[1], TexCoord_ES_in[2]);
    //fragNormal = interpolate3D(Normal_ES_in[0], Normal_ES_in[1], Normal_ES_in[2]);
    fragNormal = estimateNormal(fragTexCoord);
    fragNormal = (mbo.model * vec4(fragNormal, 1.0)).xyz;
    fragNormal = normalize(fragNormal);
    fragPosition = interpolate3D(WorldPos_ES_in[0], WorldPos_ES_in[1], WorldPos_ES_in[2]);

    // Displace the vertex along the normal
    float Displacement = texture(heightmap, fragTexCoord.xy).x;
    fragPosition.y *= Displacement;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragTangent = vec3(0);
} 