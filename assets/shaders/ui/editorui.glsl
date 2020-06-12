#version 330 core

out vec4 color;

in vec4 fragmentColor;
in vec2 fragmentTexCoord;

uniform sampler2D inTexture;

void main() {
    color = fragmentColor * texture(inTexture, fragmentTexCoord);
}