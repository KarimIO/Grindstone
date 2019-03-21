#version 330 core

layout(location = 0) out vec4 out0;
layout(location = 1) out vec4 out1;
layout(location = 2) out vec4 out2;

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragTexCoord;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

void main() {
    float height = fragPosition.y;
    /*float slope = 1.0f - normalize(fragNormal).y;

    const float snowMax = 0.95f * 10.0f;
    const float snowMin = 0.88f * 10.0f;
    const float slopeMax = 0.006;
    const float slopeMin = 0.001;

    vec4 grasscolor = vec4(0, 1, 0, 1);
    vec4 stonecolor = vec4(0.6, 0.4, 0.3, 1);

    if (height > snowMax) {
        out0 = vec4(1.0f);
    }
    else if (height > snowMin) {
        out0 = mix(grasscolor, vec4(1.0f), (height - snowMin) / (snowMax - snowMin));
    }
    else{
        out0 = grasscolor;
    }

    if (slope > slopeMax)
        out0 = stonecolor;
    else if (slope > slopeMin)
        out0 = mix(out0, stonecolor, (slope - slopeMin) / (slopeMax - slopeMin));*/

    out0 = vec4(fragTexCoord, 0.0, 1.0);
    out1 = vec4(fragNormal, 1);
    out2 = vec4(0.04, 0.04, 0.04, 0.8);
}
