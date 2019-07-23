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

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    
    float t = ubo.time / 10.0f;

    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        vec2 v = st + (i + 1) * vec2(sin((i + 1) * t));
        value += amplitude * noise(v);
        st *= 2.;
        amplitude *= .5;
    }

    value = abs(value);
    value = 1.0f - value;

    return value;
}

float getVal(vec2 uv) {
    return fbm(uv + fbm(uv + fbm(uv)));
}

vec4 getVals(vec2 uv) {
    vec2 size = vec2(0.5, 0.0);
    vec3 off = vec3(-0.1, 0, 0.1);
    
    float s11 = getVal(uv);
    float s01 = getVal(uv + off.xy);
    float s21 = getVal(uv + off.zy);
    float s10 = getVal(uv + off.yx);
    float s12 = getVal(uv + off.yz);

    vec3 va = normalize(vec3(size.xy, s21-s01));
    vec3 vb = normalize(vec3(size.yx, s12-s10));

    return vec4(cross(va, vb).xzy, s11);
}

void main() {
    vec2 uv = vertexPosition.xz / 5.0f;
    vec4 f = getVals(uv);
    vec3 p = vertexPosition + vec3(0, f.w, 0);

    // mbo.model messes up everything for some reason
    fragPosition = (mbo.model * vec4(p, 1.0)).xyz;
    gl_Position = ubo.proj_view * vec4(fragPosition, 1.0);
    fragNormal = f.xyz; //normalize((mbo.model * vec4(vertexNormal, 1.0)).xyz);
    fragTangent =  cross(vec3(0, 1, 0), fragNormal); //normalize((mbo.model * vec4(vertexTangent, 1.0)).xyz);
    fragTexCoord = vec2(vertexTexCoord.x, -vertexTexCoord.y);
}