#version 330 core

out vec4 outColor;

in vec3 fragCoord;

uniform samplerCube skybox;

void main() {
    outColor = vec4(1,0,0,1); //texture(skybox, fragCoord);
}