#version 330 core

out vec4 out_color;

in vec3 fragPosition;
in vec4 fragColor;

void main() {
    out_color = fragColor;
}
