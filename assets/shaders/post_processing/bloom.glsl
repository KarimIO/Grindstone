#version 330 core

in vec2 fragTexCoord;
out vec4 outval;

uniform sampler2D lighting;

void main() {
    outval = texture(lighting, fragTexCoord);
    outval = vec4(outval.rgb, 1);
}