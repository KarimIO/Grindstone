#version 330 core

out vec4 color;

in vec2 fragTexCoord;

void main() {
    color = vec4(UV, 0, 1);
}