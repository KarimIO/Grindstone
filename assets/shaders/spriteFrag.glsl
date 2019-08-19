#version 330 core
in vec2 fragTexCoords;
out vec4 color;

uniform sampler2D image;

layout(std140) uniform SpriteUBO {
    mat4 model;
    vec4 color;
    float aspect;
    bool is_ortho;
} subo;

void main() {
    color = vec4(subo.color.xyz, 1) * texture(image, fragTexCoords);
    color.a *= subo.color.a;
    if (color.a == 0.0f) discard;
}