#version 330 core

out vec4 outColor;

in vec3 fragCoord;

uniform samplerCube skybox;

void main() {
    outColor = pow(texture(skybox, fragCoord), vec4(2.2));
}