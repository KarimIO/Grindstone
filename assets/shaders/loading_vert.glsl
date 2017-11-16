#version 330 core

in vec2 vertexPosition;

out vec2 fragTexCoord;

void main() {
	fragTexCoord = (vertexPosition.xy + vec2(1,1)) / 2.0;
	gl_Position =  vec4(vertexPosition, 0.0, 1);
}