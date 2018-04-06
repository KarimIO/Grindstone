#version 330 core

in vec2 fragTexCoord;
out float outval;

uniform sampler2D lighting;

layout(std140) uniform ExposureUBO {
    float exposure;
} exposure_ubo;

void main() {
    vec4 light = texture(lighting, fragTexCoord);
    float lum = dot(light.rgb, vec3(0.3, 0.59, 0.11));
    outval = log(lum);
}